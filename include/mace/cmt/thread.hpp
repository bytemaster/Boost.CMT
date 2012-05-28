/**
 * @file mace/cmt/thread.hpp
 */
#ifndef MACE_CMT_THREAD_HPP
#define MACE_CMT_THREAD_HPP
#include <vector>
#include <mace/cmt/task.hpp>
#include <mace/cmt/retainable.hpp>
#include <boost/chrono.hpp>

namespace mace { namespace cmt {
   using boost::chrono::microseconds;
   using boost::chrono::system_clock;

   class abstract_thread : public retainable {
        public:
            typedef retainable_ptr<abstract_thread> ptr;

            virtual ~abstract_thread(){};
        protected:
            friend class promise_base;
            virtual void wait( const promise_base::ptr& p, const boost::chrono::microseconds& timeout_us ) = 0;
            virtual void wait( const promise_base::ptr& p, const system_clock::time_point& timeout ) = 0;
            virtual void notify( const promise_base::ptr& p ) = 0;
   };
   priority current_priority();

   /**
    * @brief manages cooperative scheduling of tasks within a single operating system thread.
    */
   class thread : public abstract_thread {
        public:
            static thread& current();

            void async( const boost::function<void()>& t, priority p = priority() );

            const char* name()const;
            void        set_name( const char* n );

            static thread* create( const char* name = ""  );


            /**
             *  Calls function @param f in this thread and returns a future<T> that can
             *  be used to wait on the result.   If async is called from a thread not currently
             *  calling exec() then the future will block on a mutex/wait condition.  If async is
             *  called from a thread running exec() then it will block 'cooperatively' and allow
             *  other tasks to run in that thread until this thread has returned a result.
             *
             *  @note Calling mace::cmt::async(...).wait() in a thread before calling exec will
             *  block that thread forever because the thread will not get a chance to process
             *  the call before blocking the thread.
             *
             *  @param when - determines when this call will happen, as soon as possible after @parm when
             */
            template<typename T>
            future<T> schedule( const boost::function<T()>& f, const system_clock::time_point& when, priority prio = priority(), const char* n= "" ) {
                   typename promise<T>::ptr p(new promise<T>());
                   task::ptr tsk( new rtask<T>(f,p,when,std::max(current_priority(),prio),n) );
                   async(tsk);
                   return p;
            }
            void schedule( const boost::function<void()>& f, const system_clock::time_point& when, priority prio = priority(), const char* n= "" ) {
                   task::ptr tsk( new vtask(f,when,std::max(current_priority(),prio),n) );
                   async(tsk);
            }




            /**
             *  Calls function @param f in this thread and returns a future<T> that can
             *  be used to wait on the result.   If async is called from a thread not currently
             *  calling exec() then the future will block on a mutex/wait condition.  If async is
             *  called from a thread running exec() then it will block 'cooperatively' and allow
             *  other tasks to run in that thread until this thread has returned a result.
             *
             *  @note Calling mace::cmt::async(...).wait() in a thread before calling exec will
             *  block that thread forever because the thread will not get a chance to process
             *  the call before blocking the thread.
             */
            template<typename T>
            future<T> async( const boost::function<T()>& f, priority prio = priority(), const char* n= "" ) {
                   typename promise<T>::ptr p(new promise<T>());
                   task::ptr tsk( new rtask<T>(f,p,std::max(current_priority(),prio),n) );
                   async(tsk);
                   return p;
            }
            template<typename T>
            T sync( const boost::function<T()>& t, priority prio, const microseconds& timeout_us=microseconds::max(), const char* n= "" ) {
                   stack_retainable<promise<T> > prom; prom.retain(); prom.retain();
                   typename promise<T>::ptr p((promise<T>*)&prom);
                   stack_retainable<rtask<T> > tsk(t,p,(std::max)(current_priority(),prio),n); tsk.retain();
                   async(&tsk);
                   return p->wait(timeout_us);
            }
            template<typename T>
            T sync( const boost::function<T()>& t, const microseconds& timeout_us=microseconds::max(), const char* n= "" ) {
                   stack_retainable<promise<T> > prom; prom.retain(); prom.retain();
                   typename promise<T>::ptr p((promise<T>*)&prom);
                   stack_retainable<rtask<T> > tsk(t,p,current_priority(),n); tsk.retain();
                   async(&tsk);
                   return p->wait(timeout_us);
            }
            void sync( const boost::function<void()>& t, const microseconds& timeout_us=microseconds::max(), const char* n= "" ) {
                   stack_retainable<promise<void> > prom; prom.retain(); prom.retain();
                   typename promise<void>::ptr p((promise<void>*)&prom);
                   stack_retainable<rtask<void> > tsk(t,p,current_priority(),n); tsk.retain();
                   async(&tsk);
                   p->wait(timeout_us);
            }


            void quit( );
            void exec();

            bool is_running()const;

            priority current_priority()const;
            ~thread();

            void set_boost_thread( boost::thread* t );
        protected:
            friend struct thread_private;
            friend void mace::cmt::yield();
            friend void mace::cmt::usleep( uint64_t );
            friend void mace::cmt::sleep_until( const system_clock::time_point& );

            // these methods may only be called from the current thread
            void yield();
            void usleep( uint64_t us );
            void sleep_until( const system_clock::time_point& tp );


            void wait( const promise_base::ptr& p, const microseconds& timeout_us );
            void wait( const promise_base::ptr& p, const system_clock::time_point& timeout );
            void notify( const promise_base::ptr& p );

        private:
            thread();

            friend class promise_base;
            void async( const task::ptr& t );
            class thread_private* my;
   };

   template<typename T>
   future<T> async( const boost::function<T()>& t, const char* n, priority prio=priority()) {
        return cmt::thread::current().async<T>(t,(std::max)(current_priority(),prio),n);
   }
   template<typename T>
   future<T> async( const boost::function<T()>& t, priority prio=priority(), const char* n = "") {
        return cmt::thread::current().async<T>(t,(std::max)(current_priority(),prio),n);
   }
   template<typename T>
   T sync( const boost::function<T()>& t, const char* n, priority prio = current_priority(), const microseconds& timeout_us = microseconds::max()) {
        return cmt::thread::current().sync<T>(t,prio,timeout_us,n);
   }
   template<typename T>
   T sync( const boost::function<T()>& t, priority prio, const microseconds& timeout_us = microseconds::max(), const char* n = "") {
        return cmt::thread::current().sync<T>(t,(std::max)(current_priority(),prio),timeout_us,n);
   }
   template<typename T>
   T sync( const boost::function<T()>& t, const microseconds& timeout_us = microseconds::max(), const char* n = "") {
        return cmt::thread::current().sync<T>(t,current_priority(),timeout_us,n);
   }
   void async( const boost::function<void()>& t, priority prio=priority() ); 
   int  exec();

   /**
    *   Sleeps the current stack for @param us microseconds.
    */
   inline void usleep( uint64_t us ) {
        mace::cmt::thread::current().usleep(us);
   }
   inline void sleep_until( const system_clock::time_point& tp ) {
        mace::cmt::thread::current().sleep_until(tp);
   }

   void yield();
} } // mace::cmt

#endif // MACE_CMT_THREAD_HPP
