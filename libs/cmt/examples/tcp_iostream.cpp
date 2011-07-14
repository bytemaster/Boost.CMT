#include <boost/cmt/asio/basic_socket_iostream.hpp>
#include <boost/cmt/thread.hpp>
namespace boost { namespace cmt { namespace asio { 
namespace tcp {
    typedef boost::cmt::asio::basic_socket_iostream< boost::asio::ip::tcp> iostream;
}
} } }

void main2() {
    boost::cmt::asio::tcp::iostream i;
    if( i.good() ) {
        slog( "ok" );
    } else {
        elog( "good" );
    }
    i.connect( boost::cmt::asio::tcp::resolve( "www.apple.com", "http" ).front() );
    if( !i )
        elog( "Unable to connect" );

    i << "GET /index.html HTTP/1.0\r\n";
    i << "HOST: www.apple.com\r\n";
    i << "Accept: */*\r\n";
    i << "Connection: close\r\n\r\n";

    std::string l;
    std::getline(i, l,'\n');
    while( i ) {
        std::cerr<<l<<std::endl;
        std::getline(i, l,'\n');
    }
}

int main( int argc, char** argv ){
    boost::cmt::async(main2);
    boost::cmt::exec();
    return 0;
}
