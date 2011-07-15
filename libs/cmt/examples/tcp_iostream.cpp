#include <boost/cmt/asio/tcp.hpp>
#include <boost/cmt/thread.hpp>

void main2() {
    boost::cmt::asio::tcp::iostream i;
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
