#include <boost/cmt/thread.hpp>
#include <boost/context/all.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/cmt/usclock.hpp>

#include <boost/cmt/log/log.hpp>

#include <list>
#include <vector>

namespace boost { namespace cmt {
    namespace bc = boost::contexts;
    struct context_t : public bc::context< bc::protected_stack > {
        typedef context_t* ptr;
        template<typename Func>
        context_t( Func f, BOOST_RV_REF(bc::protected_stack) s, bool a, bool b )
        :bc::context<>( boost::move(f), boost::move(s),a,b),next_blocked(0),next(0),prom(0){}

        context_t()
        :bc::context<>(),next_blocked(0),next(0),prom(0){}

        priority                 prio;
        promise_base*            prom; 
        uint64_t                 resume_time;
        context_t*               next_blocked;
        context_t*               next;
    };
    struct sleep_priority_less {
        bool operator()( const context_t::ptr& a, const context_t::ptr& b ) {
            return a->resume_time > b->resume_time;
        }
    };

    class thread_private {
        public:
           thread_private()
            :current(0),
             ready_head(0),
             ready_tail(0),
             blocked(0),
             task_in_queue(0)
            {}

           boost::mutex                     task_ready_mutex;
           boost::condition_variable        task_ready;

           boost::atomic<task*>             task_in_queue;
           std::vector<task*>               task_pqueue;
           std::vector<context_t*>          sleep_pqueue;

           context_t*                current;
           context_t*                ready_head;
           context_t*                ready_tail;

           context_t*                blocked;

           uint64_t check_for_timeouts();

           context_t::ptr ready_pop_front() {
                context_t::ptr tmp = 0;
                if( ready_head ) {
                    tmp        = ready_head;
                    ready_head = tmp->next;
                    if( !ready_head )   
                        ready_tail = 0;
                    tmp->next = 0;
                }
                return tmp;
           }
           void ready_push_front( const context_t::ptr& c ) {
                c->next = ready_head;
                ready_head = c;
                if( !ready_tail ) 
                    ready_tail = c;
           }
           void ready_push_back( const context_t::ptr& c ) {
                c->next = 0;
                if( ready_tail ) { 
                    ready_tail->next = c;
                } else {
                    ready_head = c;
                }
                ready_tail = c;
           }
           struct task_priority_less {
               bool operator()( const task::ptr& a, const task::ptr& b ) {
                   return a->prio.value < b->prio.value ? true :  (a->prio.value > b->prio.value ? false : a->posted_us > b->posted_us );
               }
           };
           void enqueue( const task::ptr& t ) {
                task::ptr cur = t;
                while( cur ) {
                    task_pqueue.push_back(cur);
                    std::push_heap( task_pqueue.begin(),
                                    task_pqueue.end(), task_priority_less()   );
                    cur = cur->next;
                }
           }
           task::ptr dequeue() {
                // get a new task
                task::ptr pending = task_in_queue.exchange(0,boost::memory_order_consume);
                if( pending ) {
                    enqueue( pending );
                }
                task::ptr p(0);
                if( task_pqueue.size() ) {
                    p = task_pqueue.front();
                    std::pop_heap(task_pqueue.begin(), task_pqueue.end(), task_priority_less() );
                    task_pqueue.pop_back();
                }
                return p;
           }

    };
    uint64_t thread_private::check_for_timeouts() {
        if( !sleep_pqueue.size() ) 
            return -1;

        uint64_t now = sys_clock();
        if( now < sleep_pqueue.front()->resume_time ) {
            return sleep_pqueue.front()->resume_time;
        }

        while( sleep_pqueue.size() && now >= sleep_pqueue.front()->resume_time ) {
            context_t::ptr c = sleep_pqueue.front();
            ready_push_back( c );
            std::pop_heap(sleep_pqueue.begin(), sleep_pqueue.end(), sleep_priority_less() );
            sleep_pqueue.pop_back();

            if( c->prom ) {
                c->prom->set_exception( boost::copy_exception( error::future_wait_timeout() ) );
                BOOST_ASSERT(c->prom == NULL );
            }
        }
        return 0;
    }

    thread& thread::current() {
#ifdef _MSC_VER
		static __declspec(thread) thread* t = NULL;
#else
        static __thread thread* t = NULL;
#endif
		if( !t ) t = new thread();
        return *t;
    }

    void start_thread( const promise<thread*>::ptr p  )
    {
        p->set_value( &thread::current() );
        exec();
    }

    thread* thread::create() {
        promise<thread*>::ptr p(new promise<thread*>());
        new boost::thread( boost::bind(start_thread,p) );
        return p->wait();
    }


    int  exec() { cmt::thread::current().exec(); return 0; }
    void async( const boost::function<void()>& t, priority prio ) {
       thread::current().async(t,prio);
    }

    thread::thread() {
        my = new thread_private();
    }

    thread::~thread() {
        delete my;
    }

    void thread::usleep( uint64_t timeout_us ) {
        //slog( "usleep %1%", timeout_us );
        BOOST_ASSERT( &current() == this );
        BOOST_ASSERT(my->current);

        my->current->resume_time = sys_clock() + timeout_us;
        my->current->prom = 0;

        my->sleep_pqueue.push_back(my->current);
        std::push_heap( my->sleep_pqueue.begin(),
                        my->sleep_pqueue.end(), sleep_priority_less()   );

        context_t*  prev = my->current;
        my->current = 0;
        prev->suspend();
        my->current = prev;
        my->current->resume_time = -1;
        my->current->prom = 0;
    }

    void thread::wait( const promise_base::ptr& p, uint64_t timeout_us ) {
        BOOST_ASSERT( &current() == this );
        BOOST_ASSERT( !p->ready() );
        BOOST_ASSERT(my->current);

        if( timeout_us != -1 ) {
            my->current->resume_time = sys_clock() + timeout_us;
            my->current->prom = p.get();
            my->sleep_pqueue.push_back(my->current);
            std::push_heap( my->sleep_pqueue.begin(),
                            my->sleep_pqueue.end(), sleep_priority_less()   );
        }

        context_t* tmp = my->current;
        my->current = NULL;

        tmp->prom         = p.get();
        tmp->next_blocked = my->blocked;
        my->blocked = tmp;

        tmp->suspend();
        tmp->prom = 0;
    }

    void thread::notify( const promise_base::ptr& p ) {
        BOOST_ASSERT(p->ready());
        if( &current() != this )  {
            async( boost::bind( &thread::notify, this, p ) );
            return;
        }
        //slog( "notify! %1%  ready %2%", p.get(), p->ready() );
        context_t* cur_blocked  = my->blocked;
        context_t* prev_blocked = 0;
        while( cur_blocked ) {
            if( cur_blocked->prom == p.get() ) {
                if( prev_blocked ) { 
                    prev_blocked->next_blocked = cur_blocked->next_blocked; 
                } else { 
                    my->blocked = cur_blocked->next_blocked; 
                }
                cur_blocked->next_blocked = 0;
                //cur_blocked->prom         = 0;
                my->ready_push_front( cur_blocked );


                // stop searching after waking one task
                cur_blocked = 0;
            } else {
                prev_blocked  = cur_blocked;
                cur_blocked   = cur_blocked->next_blocked;
            }
        }
        for( uint32_t i = 0; i < my->sleep_pqueue.size(); ++i ) {
            if( my->sleep_pqueue[i]->prom == p.get() ) {
                my->sleep_pqueue[i]->prom = 0;
                my->sleep_pqueue[i] = my->sleep_pqueue.back();
                my->sleep_pqueue.pop_back();
                std::make_heap( my->sleep_pqueue.begin(),my->sleep_pqueue.end(), sleep_priority_less() );
                break;
            }
        }
    }

    void thread::exec_fiber( ){
        BOOST_ASSERT( my->current );
        while( true ) {
            while( my->ready_head  ) {
                context_t* cur  = my->current;
                context_t* next = my->ready_pop_front();
                my->current = next;
                next->resume();

                BOOST_ASSERT( !next->is_complete() );

                my->current = cur;
                my->check_for_timeouts();
            }

            task::ptr next = my->dequeue();
            if( !next ) {
                uint64_t timeout_time = my->check_for_timeouts();
                if( timeout_time == 0 )
                    continue;
                boost::unique_lock<boost::mutex> lock(my->task_ready_mutex);
                if( !(next = my->dequeue())) {
                    if( timeout_time == -1 ) {
                        slog( "wait..." );
                        my->task_ready.wait( lock );
                    } else {
                        slog( "timeout_time %1%   to_ptime %2%", timeout_time, 
                            (to_ptime(timeout_time)-boost::get_system_time()).total_microseconds() );
                        my->task_ready.timed_wait( lock, to_ptime(timeout_time) );
                    }
                    continue;
                }
            }
            BOOST_ASSERT(next);
            if( my->current ) 
                my->current->prio = next->prio;
            next->run();
            next->release();
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
                while( my->ready_head ) {
                    context_t* next = my->ready_pop_front();
                    next->resume();
                    if( next->is_complete() ) {
                      // elog( "next complete!!" ); 
                       delete next;
                    }
                    my->current = 0;
                    my->check_for_timeouts();
                }
                //wlog( "creating new context" );
                // create a new coro if there are no ready tasks
                context_t* new_context = new context_t( boost::bind(&thread::exec_fiber,this), 
                                                 bc::protected_stack( bc::stack_helper::default_stacksize() ), true, true );
                my->current = new_context;
                my->current->resume(); 
                delete my->current;
                my->current = 0;
            }
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
     */
    void thread::yield() {
        if( my->current ) {
            context_t*  prev = my->current;
            my->ready_push_back(my->current);
            my->current = 0;
            my->ready_tail->suspend();
            my->current = prev;
        }
    }

    void thread::quit() {
        if( &current() != this ) {
            async( boost::bind( &thread::quit, this ) );
        }
        context_t* cur  = my->blocked;
        while( cur ) {
            // setting the exception, will notify, thus modify blocked list
            cur->prom->set_exception( boost::copy_exception( error::thread_quit() ) );
            cur = my->blocked;
        }
    }


    void thread::async( const boost::function<void()>& t, priority prio ) {
       async(task::ptr( new vtask(t,(std::max)(current_priority(),prio)) ) );
    }
    void thread::async( const task::ptr& t ) {
        //slog( "async..." );
        task::ptr stale_head = my->task_in_queue.load(boost::memory_order_relaxed);
        do {
            t->next = stale_head;
        }while( !my->task_in_queue.compare_exchange_weak( stale_head, t, boost::memory_order_release ) );

        if( this != &current() ) {
            boost::unique_lock<boost::mutex> lock(my->task_ready_mutex);
            my->task_ready.notify_all();
        }
        //slog( "return.." );
    }
    void yield() { thread::current().yield(); }

    priority current_priority() { return cmt::thread::current().current_priority(); }
    priority thread::current_priority()const {
        if( my->current ) return my->current->prio;
        return priority();
    }

} } // namespace boost::cmt 

