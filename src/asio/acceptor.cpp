#include <boost/cmt/asio/acceptor.hpp>
#include <boost/cmt/asio.hpp>
#include <boost/cmt/thread.hpp>

namespace boost { namespace cmt {  namespace asio {
    class acceptor_private {
        public:
          boost::asio::ip::tcp::acceptor*              acc; 
          acceptor*                                    self;
          future<void_t>                               listen_done;
          acceptor::handler                            new_connection;

          acceptor_private( acceptor* s )
          :acc(NULL),self(s) {}

          ~acceptor_private() {
            try {
                delete acc;
                if( listen_done.valid() )
                    listen_done.wait();
            } catch ( const boost::exception& e ) {
                
            }
          }

          tcp_socket_ptr accept( uint64_t timeout_us = -1 ) { 
             tcp_socket_ptr sock( new tcp_socket( boost::cmt::asio::default_io_service() ) );
             cmt::promise<boost::system::error_code>::ptr p(new cmt::promise<boost::system::error_code>());
             acc->async_accept( *sock, boost::bind( boost::cmt::asio::detail::error_handler, p, _1 ) );
             if( boost::system::error_code ec = p->wait(timeout_us) ) {
                BOOST_THROW_EXCEPTION( boost::system::system_error(ec) );
             }
             return sock;
          }

          void_t listen( uint16_t port ) {
            try {
                if( !acc )
                    acc = new boost::asio::ip::tcp::acceptor( boost::cmt::asio::default_io_service(), 
                               boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(),port) );
                while( true ) {
                   try {
                      cmt::async( boost::bind(new_connection, accept(), boost::system::error_code() ) );
                   } catch ( const boost::system::system_error& se  ) {
                      cmt::async( boost::bind(new_connection, accept(), se.code() ) );
                   }
                }
            } catch ( const boost::exception& e ) {
                std::cerr<<"Ignoring exception: " << boost::diagnostic_information(e) << std::endl;
                throw;
            }
            return void_t();
          }
    };

    acceptor::acceptor() {
        my = new acceptor_private(this);
    }
    acceptor::~acceptor() {
        delete my;
    }

    void acceptor::listen( uint16_t port, const acceptor::handler& on_con ) {
        my->new_connection = on_con;
        my->listen_done = cmt::async<void_t>( boost::bind( &acceptor_private::listen, my, port ) );
    }


} } } // boost::cmt::asio
