#include <boost/cmt/thread.hpp>
#include <boost/context/all.hpp>
#include <boost/lockfree/fifo.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include <boost/cmt/log/log.hpp>

#include <list>
#include <vector>

namespace boost { namespace cmt {
    namespace bc = boost::contexts;
    typedef bc::context<> context_t;

    typedef std::map<promise_base::ptr, std::vector<context_t*> >  blocked_map;
    typedef std::map<promise_base::ptr, boost::posix_time::ptime > prom_to_timeout_map;
    typedef std::map<boost::posix_time::ptime, promise_base::ptr > timeout_to_prom_map;
    typedef std::map<boost::posix_time::ptime, context_t* >        sleeping_map;

    class thread_private {
        public:
           boost::mutex                     task_ready_mutex;
           boost::condition_variable        task_ready;
           boost::lockfree::fifo<task::ptr> task_queue;  

           context_t*                current;
           std::list< context_t* >   ready;
           std::vector< context_t* > idle;

           blocked_map               blocked;
           sleeping_map              sleeping;

           prom_to_timeout_map  prom_to_timeout;
           timeout_to_prom_map  timeout_to_prom;

           boost::posix_time::ptime check_for_timeouts();
    };
    boost::posix_time::ptime thread_private::check_for_timeouts() {
       boost::posix_time::ptime now = boost::get_system_time();

       bool found = false;

       sleeping_map::iterator sitr = sleeping.begin();
       while( sitr != sleeping.end() && sitr->first <= now ) {
            ready.push_back(sitr->second);
            sleeping.erase(sitr);
            sitr = sleeping.begin();
            found = true;
       }

       timeout_to_prom_map::iterator itr = timeout_to_prom.begin();
       while( itr != timeout_to_prom.end() && now > itr->first ) {
            // set_exception will call notify which will free the map entries
            itr->second->set_exception( boost::copy_exception( error::future_wait_timeout() ) );
            itr = timeout_to_prom.begin();
            found = true;
       }
       if( found )
            return boost::get_system_time() - boost::posix_time::seconds(60*60*24);
       boost::posix_time::ptime next_timeout;

       if( sitr != sleeping.end() && itr != timeout_to_prom.end() ) {
            return (std::min)(sitr->first,itr->first);
       }
       if( itr != timeout_to_prom.end() )
            return itr->first;
       if( sitr != sleeping.end() )
            return sitr->first;

       return boost::get_system_time() + boost::posix_time::seconds(60*60*24);
    }

    thread& thread::current() {
        static __thread thread* t = NULL;
        if( !t ) t = new thread();
        return *t;
    }

    void start_thread( const promise<thread*>::ptr p  )
    {
        p->set_value( &thread::current() );
    }

    thread* create() {
        promise<thread*>::ptr p(new promise<thread*>());
        new boost::thread( boost::bind(start_thread,p) );
        return p->wait();
    }


    int  exec() { cmt::thread::current().exec(); return 0; }
    void async( const boost::function<void()>& t ) {
       thread::current().async(t);
    }

    thread::thread() {
        my = new thread_private();
    }

    thread::~thread() {
        delete my;
    }

    void thread::usleep( uint64_t timeout_us ) {
        BOOST_ASSERT( my->current );
        BOOST_ASSERT( &current() == this );

        my->sleeping[boost::get_system_time()+boost::posix_time::microseconds(timeout_us)] = my->current;
        context_t*  prev = my->current;
        my->current = 0;
        prev->suspend();
        my->current = prev;
    }

    void thread::wait( const promise_base::ptr& p, uint64_t timeout_us ) {
        BOOST_ASSERT( &current() == this );
        BOOST_ASSERT(my->current);

        if( timeout_us != -1 ) {
            boost::posix_time::ptime to = boost::get_system_time() + boost::posix_time::microseconds(timeout_us);

            my->timeout_to_prom[to] = p;
            my->prom_to_timeout[p] = to;
        }

        context_t* tmp = my->current;
        my->current = NULL;
        my->blocked[p].push_back( tmp );
        tmp->suspend();
    }

    void thread::notify( const promise_base::ptr& p ) {
        if( &current() != this )  {
            async( boost::bind( &thread::notify, this, p ) );
            return;
        }
        blocked_map::iterator itr = my->blocked.find( p );
        if( itr != my->blocked.end() ) {
            for( uint32_t i = 0; i < itr->second.size(); ++i ) {
                // TODO: priority sort as we push
                my->ready.push_front( itr->second[i] );
                boost::unique_lock<boost::mutex> lock(my->task_ready_mutex);
                my->task_ready.notify_all();
            }
            my->blocked.erase(itr);
            my->timeout_to_prom.erase(my->prom_to_timeout[p]);
            my->prom_to_timeout.erase(p);
        }
    }


    /**
     *  If no current context, create a new context running exec()
     *  If a current context exists, process tasks until the queue is
     *  empty and then block.
     */
    void thread::exec() {
        if( !my->current ) {
            while( true ) {
                // run ready tasks
                while( my->ready.size() ) {
                    context_t* next = my->ready.front();
                    my->ready.pop_front();
                    next->resume();
                    my->current = 0;
                    my->check_for_timeouts();
                }
                wlog( "creating new context" );
                // create a new coro if there are no ready tasks
                context_t* new_context = new context_t( boost::bind(&thread::exec,this), 
                                                 bc::protected_stack( bc::stack_helper::default_stacksize() ), true, true );
                my->current = new_context;
                my->current->resume(); 
            }
        }
        while( true ) {
 
            // all currently active & ready contexts get priority over new
            // posted tasks.  
            while( my->ready.size() ) {
                context_t* cur  = my->current;
                context_t* next = my->ready.front();
                my->ready.pop_front();
                my->current = next;
                next->resume();
                my->current = cur;
                my->check_for_timeouts();
            }

            // get a new task
            task::ptr next;
            if( !my->task_queue.dequeue( &next ) ) {
                boost::posix_time::ptime timeout_time = my->check_for_timeouts();
                if( timeout_time < boost::get_system_time() ) 
                    continue;
                boost::unique_lock<boost::mutex> lock(my->task_ready_mutex);
                if( !my->task_queue.dequeue( &next ) ) {
                    my->task_ready.timed_wait( lock, timeout_time );
                    continue;
                }
            }
            //slog( "run task %1% in context %2%", next->name(), my->current );
            next->run();
           // slog( "return from %1%", next->name() );
            delete next;
        }
   }

    /**
     *   Switches from the current task to the next ready task.
     *
     *   If there are no other ready tasks and the input queue is empty then
     *   return immediately.  
     *
     *   If there are no other ready contexts, but items on the input queue then push
     *   this context into the ready queue, mark current as NULL and then
     *   start a new context running exec() to process the next item on the
     *   input queue. Yield will return next time a context yields. 
     *
     *   If there are other ready contexts, push this context on to the end and then
     *   yield to the first ready context.  Yield will return after all other ready
     *   tasks have yielded at least once.
     *
     *   If yield is called with no current context, then it runs exec()
     */
    void thread::yield() {
        BOOST_ASSERT( my->current );

        context_t*  prev = my->current;
        my->ready.push_back(my->current);
        my->current = 0;
        my->ready.back()->suspend();
        my->current = prev;
    }

    void thread::quit() {
        if( &current() != this ) {
            async( boost::bind( &thread::quit, this ) );
        }
        blocked_map::iterator itr = my->blocked.begin();
        while( itr != my->blocked.end() ) {
            // setting the exception, will notify 
            itr->first->set_exception( boost::copy_exception( error::thread_quit() ) );
            itr = my->blocked.begin();
        }
       
    }


    void thread::async( const boost::function<void()>& t ) {
       async(task::ptr( new vtask(t) ) );
    }
    void thread::async( const task::ptr& t ) {
        if( my->task_queue.enqueue( t ) ) {
            if( this != &current() ) {
                boost::unique_lock<boost::mutex> lock(my->task_ready_mutex);
                my->task_ready.notify_all();
            }
        }
    }


} } // namespace boost::cmt 

