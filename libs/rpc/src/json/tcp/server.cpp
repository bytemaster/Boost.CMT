#include <boost/rpc/json/tcp/connection.hpp>
#include <boost/asio.hpp>

namespace boost { namespace rpc { namespace json { namespace tcp {
    typedef boost::cmt::asio::tcp::iostream iostream;
    typedef boost::function<void(const connection::ptr&)> handler;

    namespace detail {
        void listen( uint16_t port, const handler& handle ) {
          boost::asio::ip::tcp::acceptor acc( boost::cmt::asio::default_io_service(), 
                                              boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(),port) );
         
          boost::system::error_code ec;
          do {
              iostream::ptr iosp(new iostream());
              ec = boost::cmt::asio::tcp::accept( acc, *(iosp->rdbuf()) );
              if(!ec) {
                  boost::cmt::async( boost::bind(handle, connection::ptr( new connection(iosp) ) )); 
              }
          }while( !ec );
          handle(connection::ptr());
        }
    }

    void listen( uint16_t port, const handler& handle ) {
        boost::cmt::async( boost::bind( &detail::listen, port, handle ) );
    }

} } } } // boost::rpc::json::tcp
