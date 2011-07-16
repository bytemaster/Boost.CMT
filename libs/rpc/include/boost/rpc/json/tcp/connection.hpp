#ifndef _BOOST_RPC_JSON_CONNECTION_HPP_
#define _BOOST_RPC_JSON_CONNECTION_HPP_
#include <boost/rpc/json.hpp>
#include <boost/cmt/thread.hpp>
#include <boost/cmt/asio/tcp.hpp>
#include <boost/cmt/mutex.hpp>

namespace boost { namespace rpc { namespace json { namespace tcp {

    class connection : public boost::cmt::retainable {
        public:
            typedef boost::cmt::retainable_ptr<connection> ptr;
            typedef boost::cmt::asio::tcp::iostream::ptr   sock_ios_ptr;

            connection( const sock_ios_ptr& s );
            connection(){}

            bool connect( const std::string& hostname, const std::string& port );

            void send( const js::Value& v );
            void set_recv_handler( const boost::function<void(const js::Value& v)>& recv );

        private:
            void read_loop();
            boost::function<void(const js::Value& v)> m_recv_handler;
            boost::cmt::asio::tcp::iostream::ptr      m_sock_ios;
            boost::cmt::mutex                         m_mutex;
    };

} } } } // boost::rpc::json::tcp

#endif
