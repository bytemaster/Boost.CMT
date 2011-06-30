#ifndef _BOOST_CMT_FUTURE_HPP
#define _BOOST_CMT_FUTURE_HPP
#include <boost/enable_shared_from_this.hpp>
#include <boost/cmt/error.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/optional.hpp>

namespace boost { namespace cmt {

    class abstract_thread;
    class promise_base :  public boost::enable_shared_from_this<promise_base> {
         public:
             typedef boost::shared_ptr<promise_base> ptr;
             promise_base():m_blocked_thread(0){}
             virtual ~promise_base(){}

             virtual bool ready()const = 0;
         protected:
             void enqueue_thread();
             void wait( uint64_t timeout_us );
             void notify();
             virtual void set_timeout()=0;
             virtual void set_exception( const boost::exception_ptr& e )=0;

         private:
             friend class thread;
             friend class thread_private;
             abstract_thread* m_blocked_thread;
    };

    struct void_t {};

    template<typename T = void_t>
    class promise : public promise_base {
        public:
            typedef boost::shared_ptr<promise> ptr;

            promise(){}
            promise( const T& v ):m_value(v){}

            bool ready()const { 
                boost::unique_lock<boost::mutex> lock( m_mutex );
                return m_value || m_error; 
            }
            operator const T&()const  { return wait();  }

            const T& wait(uint64_t timeout = -1) {
                { // lock while we check values
                    boost::unique_lock<boost::mutex> lock( m_mutex );
                    if( m_error ) boost::rethrow_exception(m_error);
                    if( m_value ) return *m_value;
                    enqueue_thread();
                } // unlock before yielding, but after enqueing
                promise_base::wait(timeout);
                if( m_error ) boost::rethrow_exception(m_error);
                if( m_value ) return *m_value;
                BOOST_THROW_EXCEPTION( error::future_value_not_ready() ); 
                return *m_value;
            }
            void set_exception( const boost::exception_ptr& e ) {
                {
                    boost::unique_lock<boost::mutex> lock( m_mutex );
                    m_error = e;
                }
                notify();
            }
            void set_value( const T& v ) {
                {
                    boost::unique_lock<boost::mutex> lock( m_mutex );
                    if( m_error ) 
                        return;
                    m_value = v;
                }
                notify();
            }
            
        private:
            void set_timeout() {
                {
                    boost::unique_lock<boost::mutex> lock( m_mutex );
                    if( m_value ) 
                        return;
                    m_error = boost::copy_exception( error::future_wait_timeout() );
                }
                notify();
            }

            mutable boost::mutex    m_mutex;
            boost::exception_ptr    m_error;
            boost::optional<T>      m_value;
    };

    template<>
    class promise<void> : public promise<void_t> {
    };

    template<typename T = void_t>
    class future {
        public:
            future( const typename promise<T>::ptr& p = typename promise<T>::ptr() )
            :m_prom(p){}

            bool     ready()const                { return m_prom->ready();       }
            operator const T&()const             { return m_prom->wait();        }
            const T& wait(uint64_t timeout = -1) { return m_prom->wait(timeout); }

        private:
            typename promise<T>::ptr m_prom;
    };

    template<>
    class future<void> : public future<void_t> {
    };


} }


#endif
