#include <boost/rpc/json/connection.hpp>
#include <json_spirit/json_spirit_stream_reader.h>

namespace boost { namespace rpc { namespace json {
    connection::connection( const connection::sock_ios_ptr& p )
    :m_sock_ios(p) {
       boost::cmt::async( boost::bind(&connection::read_loop,this) );
    }

    bool connection::connect( const std::string& hostname, const std::string& port ) {
        m_sock_ios = sock_ios_ptr( new boost::cmt::asio::tcp::iostream() );
        m_sock_ios->connect( boost::cmt::asio::tcp::resolve( hostname, port ).front() );
        return *m_sock_ios;
    }

    void connection::send( const js::Value& v ) {
       json_spirit::write(v, *m_sock_ios);
    }

    void connection::set_recv_handler( const boost::function<void(const js::Value& v )>& v ) {
        m_recv_handler = v;
        async( boost::bind( &connection::read_loop, this ) );
    }

    void connection::read_loop() {
        js::Value v;
        js::Stream_reader<boost::cmt::asio::tcp::iostream, js::Value> reader(*m_sock_ios);
        while( reader.read_next(v) ) {
            m_recv_handler(v);
        }
    }
} } } 
