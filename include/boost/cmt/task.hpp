#ifndef BOOST_CMT_TASK_HPP
#define BOOST_CMT_TASK_HPP
#include <boost/enable_shared_from_this.hpp>
#include <boost/cmt/error.hpp>
#include <boost/cmt/retainable.hpp>
#include <boost/cmt/future.hpp>
#include <boost/function.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/cmt/log/log.hpp>
#include <boost/cmt/usclock.hpp>

namespace boost { namespace cmt {
     struct priority {
       explicit priority( int v = 0):value(v){}
       priority( const priority& p ):value(p.value){}
       bool operator < ( const priority& p )const {
        return value < p.value;
       }
       int value;
     };
    class task : public retainable {
        public:
            typedef task* ptr;
            task(priority p=priority()):posted_us(usclock()),prio(p),next(0){}

            virtual void run() = 0;
            virtual void cancel() = 0;
            virtual const char* name() { return "unknown"; }
        protected:
            friend class thread;
            friend class thread_private;
            uint64_t     posted_us;
            priority     prio;
            task*        next;
    };

    template<typename R = void>
    class rtask : public task {
        public:
            rtask( const boost::function<R()>& f, const typename promise<R>::ptr& p, priority prio, const char* name = "" )
            :task(prio),m_functor(f),m_prom(p),m_name(name){}

            void cancel() {
                try {
                    BOOST_THROW_EXCEPTION( error::task_canceled() );
                } catch ( ... ) {
                    m_prom->set_exception(boost::current_exception());
                }
            }
            void run() {
                try {
                    m_prom->set_value( m_functor() );
                } catch( ... ) {
                    m_prom->set_exception(boost::current_exception());
                }
            }
            const char* name() { return m_name; }

            boost::function<R()>        m_functor;
            typename promise<R>::ptr    m_prom;
            const char*                 m_name;
    };

    template<>
    class rtask<void> : public task {
        public:
            rtask( const boost::function<void()>& f, const  promise<void>::ptr& p, priority prio=priority(), const char* name = "" )
            :task(prio),m_functor(f),m_prom(p),m_name(name){}

            void cancel() {
                try {
                    BOOST_THROW_EXCEPTION( error::task_canceled() );
                } catch ( ... ) {
                    m_prom->set_exception(boost::current_exception());
                }
            }
            void run() {
                try {
                    m_functor();
                    m_prom->set_value( void_t() );
                } catch( ... ) {
                    m_prom->set_exception(boost::current_exception());
                }
            }
            const char* name() { return m_name; }
        
            boost::function<void()>     m_functor;
            promise<void>::             ptr m_prom;
            const char*                 m_name;

    };

    template<typename R = void_t>
    class reftask : public task {
        public:
            reftask( const boost::function<R()>& f, const typename promise<R>::ptr& p, priority prio =priority(), const char* name = "" )
            :task(prio),m_functor(f),m_prom(p),m_name(name){}

            void cancel() {
                try {
                    BOOST_THROW_EXCEPTION( error::task_canceled() );
                } catch ( ... ) {
                    m_prom->set_exception(boost::current_exception());
                }
            }
            void run() {
                try {
                    m_prom->set_value( m_functor() );
                } catch( ... ) {
                    m_prom->set_exception(boost::current_exception());
                }
            }
            const char* name() { return m_name; }

            const char*                 m_name;
            const boost::function<R()>& m_functor;
            typename promise<R>::ptr    m_prom;
    };
    template<>
    class reftask<void> : public task {
            reftask( const boost::function<void()>& f, const  promise<void>::ptr& p, priority prio=priority(),const char* name = "" )
            :task(prio),m_functor(f),m_prom(p),m_name(name){}

            void cancel() {
                try {
                    BOOST_THROW_EXCEPTION( error::task_canceled() );
                } catch ( ... ) {
                    m_prom->set_exception(boost::current_exception());
                }
            }
            void run() {
                try {
                    m_functor();
                    m_prom->set_value( void_t());
                } catch( ... ) {
                    m_prom->set_exception(boost::current_exception());
                }
            }
            const char* name() { return m_name; }

            const boost::function<void()>& m_functor;
            promise<void>::ptr              m_prom;
            const char*                    m_name;
    };


    class vtask : public task {
        
        public:
        vtask( const boost::function<void()>& f, priority prio = priority() )
        :task(prio),m_functor(f){
        }

        void cancel() {}
        void run() {
            try {
                m_functor();
            } catch( const boost::exception& e ) {
                elog( "%1%", boost::diagnostic_information(e) );
            } catch( const std::exception& e ) {
                elog( "%1%", boost::diagnostic_information(e) );
            } catch( ... ) {
                BOOST_ASSERT(!"unhandled exception");
            }
        }
        boost::function<void()> m_functor;
    };

} } // namespace boost cmt

#endif // BOOST_CMT_TASK_HPP
