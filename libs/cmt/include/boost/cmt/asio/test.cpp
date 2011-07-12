#include <boost/cmt/asio/basic_socket_iostream.hpp>
namespace boost { namespace cmt { namespace asio { 
namespace tcp {
    typedef boost::cmt::asio::basic_socket_iostream< boost::asio::ip::tcp> iostream;
}
} } }
int main( int argc, char** argv ){
    boost::cmt::asio::tcp::iostream i;
}
