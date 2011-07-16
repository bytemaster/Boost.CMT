#include <boost/rpc/json/tcp/connection.hpp>
#include <json_spirit/json_spirit_stream_reader.h>
#include <boost/thread/mutex.hpp>

namespace boost { namespace rpc { namespace json { namespace tcp {
    connection::connection( const connection::sock_ios_ptr& p )
    :m_sock_ios(p) {
    }

    bool connection::connect( const std::string& hostname, const std::string& port ) {
        m_sock_ios = sock_ios_ptr( new boost::cmt::asio::tcp::iostream() );
        m_sock_ios->connect( boost::cmt::asio::tcp::resolve( hostname, port ).front() );
        return *m_sock_ios;
    }

    void connection::send( const js::Value& v ) {
       std::cerr<<"send:"<<std::endl;
       json_spirit::write(v,std::cerr);
       boost::unique_lock<boost::cmt::mutex> lock(m_mutex);
       json_spirit::write(v, *m_sock_ios);
       m_sock_ios->flush();
    }

    void connection::set_recv_handler( const boost::function<void(const js::Value& v )>& v ) {
        m_recv_handler = v;
        async( boost::bind( &connection::read_loop, this ) );
    }

    void connection::read_loop() {
        js::Value v;
        js::Stream_reader<boost::cmt::asio::tcp::iostream, js::Value> reader(*m_sock_ios);
        while( reader.read_next(v) ) {
            std::cerr<<"recv:";
            json_spirit::write(v,std::cerr);
            std::cerr<<std::endl;
            m_recv_handler(v);
            v = js::Value();
        }
        elog( "error reading next" );
    }
} } } }  // boost::rpc::json::tcp
