#ifndef BOOST_CMT_HPP
#define BOOST_CMT_HPP
#include <vector>
#include <boost/cmt/task.hpp>
#include <boost/cmt/retainable.hpp>

namespace boost { namespace cmt {
   class abstract_thread : public retainable {
        public:
            typedef retainable_ptr<abstract_thread> ptr;

            virtual ~abstract_thread(){};
        protected:
            friend class promise_base;
            virtual void wait( const promise_base::ptr& p, uint64_t timeout_us ) = 0;
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

            static thread* create();

            template<typename T>
            future<T> async( const boost::function<T()>& t, priority prio = priority(), const char* n= "" ) {
               typename promise<T>::ptr p(new promise<T>());
               task::ptr tsk( new rtask<T>(t,p,std::max(current_priority(),prio),n) );
               async(tsk);
               return p;
            }
            template<typename T>
            T sync( const boost::function<T()>& t, priority prio, uint64_t timeout_us=-1, const char* n= "" ) {
               stack_retainable<promise<T> > prom; prom.retain(); prom.retain();
               typename promise<T>::ptr p((promise<T>*)&prom);
               stack_retainable<rtask<T> > tsk(t,p,(std::max)(current_priority(),prio),n); tsk.retain();
               async(&tsk);
               return p->wait(timeout_us);
            }
            template<typename T>
            T sync( const boost::function<T()>& t, uint64_t timeout_us=-1, const char* n= "" ) {
               stack_retainable<promise<T> > prom; prom.retain(); prom.retain();
               typename promise<T>::ptr p((promise<T>*)&prom);
               stack_retainable<rtask<T> > tsk(t,p,current_priority(),n); tsk.retain();
               async(&tsk);
               return p->wait(timeout_us);
            }

            void yield();
            void usleep( uint64_t us );

            void quit( );
            void exec();

            priority current_priority()const;
        protected:
            void wait( const promise_base::ptr& p, uint64_t timeout_us );
            void notify( const promise_base::ptr& p );
            void exec_fiber();

        private:
            thread();
            ~thread();

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
   T sync( const boost::function<T()>& t, const char* n, priority prio = current_priority(), uint64_t timeout_us = -1) {
        return cmt::thread::current().sync<T>(t,prio,timeout_us,n);
   }
   template<typename T>
   T sync( const boost::function<T()>& t, priority prio, uint64_t timeout_us = -1, const char* n = "") {
        return cmt::thread::current().sync<T>(t,(std::max)(current_priority(),prio),timeout_us,n);
   }
   template<typename T>
   T sync( const boost::function<T()>& t, uint64_t timeout_us = -1, const char* n = "") {
        return cmt::thread::current().sync<T>(t,current_priority(),timeout_us,n);
   }
   void async( const boost::function<void()>& t, priority prio=priority() ); 
   int  exec();

   /**
    *   Sleeps the current stack for @param us microseconds.
    */
   inline void usleep( uint64_t us ) {
        boost::cmt::thread::current().usleep(us);
   }

   void yield();
} } // boost::cmt

#endif
