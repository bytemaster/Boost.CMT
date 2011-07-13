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

   class thread : public abstract_thread {
        public:
            static thread& current();

            void async( const boost::function<void()>& t );

            static thread* create();

            template<typename T>
            future<T> async( const boost::function<T()>& t, const char* n= "" ) {
               typename promise<T>::ptr p(new promise<T>());
               task::ptr tsk( new rtask<T>(t,p,n) );
               async(tsk);
               return p;
            }
            template<typename T>
            T sync( const boost::function<T()>& t, uint64_t timeout_us, const char* n= "" ) {
               stack_retainable<promise<T> > prom; prom.retain(); prom.retain();
               typename promise<T>::ptr p((promise<T>*)&prom);
               stack_retainable<rtask<T> > tsk(t,p,n); tsk.retain();
               async(&tsk);
               return p->wait(timeout_us);
            }
            template<typename T>
            T sync( const boost::function<T()>& t, const char* n= "" ) {
               stack_retainable<promise<T> > prom; prom.retain();
               typename promise<T>::ptr p((promise<T>*)&prom,true);
               stack_retainable<reftask<T> > tsk(t,p,n); tsk.retain();
               async(&tsk);
               return p->wait();
            }

            void yield();
            void usleep( uint64_t us );

            void quit( );
            void exec();

        protected:
            void wait( const promise_base::ptr& p, uint64_t timeout_us );
            void notify( const promise_base::ptr& p );
            void exec_until( const promise_base::ptr& p );

        private:
            thread();
            ~thread();

            friend class promise_base;
            void async( const task::ptr& t );
            class thread_private* my;
   };

   template<typename T>
   future<T> async( const boost::function<T()>& t, uint64_t timeout, const char* n = "") {
        return cmt::thread::current().async<T>(t,timeout,n);
   }
   template<typename T>
   future<T> async( const boost::function<T()>& t, const char* n = "") {
        return cmt::thread::current().async<T>(t,n);
   }
   template<typename T>
   T sync( const boost::function<T()>& t, const char* n = "") {
        return cmt::thread::current().sync<T>(t,n);
   }
   void async( const boost::function<void()>& t ); 
   int  exec();

   void yield();
} } // boost::cmt

#endif
