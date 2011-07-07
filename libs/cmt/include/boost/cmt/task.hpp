#ifndef BOOST_CMT_TASK_HPP
#define BOOST_CMT_TASK_HPP
#include <boost/enable_shared_from_this.hpp>
#include <boost/cmt/error.hpp>
#include <boost/cmt/retainable.hpp>
#include <boost/cmt/future.hpp>
#include <boost/function.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/cmt/log/log.hpp>

namespace boost { namespace cmt {
    class task : public retainable {
        public:
            typedef task* ptr;
            task():next(0),priority(0){}

            virtual void run() = 0;
            virtual void cancel() = 0;
            virtual const char* name() { return "unknown"; }
        protected:
            friend class thread;
            friend class thread_private;
            int          priority;
            task*        next;
    };

    template<typename R = void>
    class rtask : public task {
        public:
            rtask( const boost::function<R()>& f, const typename promise<R>::ptr& p, const char* name = "" )
            :m_functor(f),m_prom(p),m_name(name){}

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
            boost::function<R()>        m_functor;
            typename promise<R>::ptr    m_prom;
    };

    template<>
    class rtask<void> : public task {
        public:
            rtask( const boost::function<void()>& f, const typename promise<void>::ptr& p, const char* name = "" )
            :m_functor(f),m_prom(p),m_name(name){}

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

            const char*                 m_name;
            boost::function<void()>     m_functor;
            typename promise<void>::ptr m_prom;
    };

    template<typename R = void_t>
    class reftask : public task {
        public:
            reftask( const boost::function<R()>& f, const typename promise<R>::ptr& p, const char* name = "" )
            :m_functor(f),m_prom(p),m_name(name){}

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


    class vtask : public task {
        
        public:
        vtask( const boost::function<void()>& f )
        :m_functor(f){}

        void cancel() {}
        void run() {
            try {
                m_functor();
            } catch( const boost::exception& e ) {
                elog( "%1%", boost::diagnostic_information(e) );
            } catch( ... ) {
                assert( !"unhandled exception" );
            }
        }
        boost::function<void()> m_functor;
    };

} } // namespace boost cmt

#endif // BOOST_CMT_TASK_HPP
