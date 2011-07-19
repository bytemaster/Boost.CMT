#ifndef _BOOST_CMT_ASIO_HPP_
#define _BOOST_CMT_ASIO_HPP_
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/cmt/future.hpp>

namespace boost { namespace cmt { namespace asio {
    namespace detail {
        using namespace boost::cmt;

        void read_write_handler( const promise<size_t>::ptr& p, 
                                 const boost::system::error_code& ec, 
                                size_t bytes_transferred );
        void read_write_handler_ec( promise<size_t>* p, 
                                    boost::system::error_code* oec, 
                                    const boost::system::error_code& ec, 
                                    size_t bytes_transferred );

        void error_handler( const promise<boost::system::error_code>::ptr& p, 
                              const boost::system::error_code& ec );
        void error_handler_ec( promise<boost::system::error_code>* p, 
                              const boost::system::error_code& ec ); 

        template<typename EndpointType, typename IteratorType>
        void resolve_handler( 
                             const typename promise<std::vector<EndpointType> >::ptr& p,
                             const boost::system::error_code& ec, 
                             IteratorType itr) {
            if( !ec ) {
                std::vector<EndpointType> eps;
                while( itr != IteratorType() ) {
                    eps.push_back(*itr);
                    ++itr;
                }
                p->set_value( eps );
            } else {
                p->set_exception( boost::copy_exception( boost::system::system_error(ec) ) );
            }
        }
    }

    boost::asio::io_service& default_io_service();

    template<typename AsyncReadStream, typename MutableBufferSequence>
    size_t read( AsyncReadStream& s, const MutableBufferSequence& buf, uint64_t timeout_us = -1 ) {
        promise<size_t>::ptr p(new promise<size_t>());
        boost::asio::async_read( s, buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p->wait(timeout_us);
    }
    template<typename AsyncReadStream, typename MutableBufferSequence>
    size_t read_some( AsyncReadStream& s, const MutableBufferSequence& buf, uint64_t timeout_us = -1 ) {
        promise<size_t>::ptr p(new promise<size_t>());
        s.async_read_some( buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p->wait(timeout_us);
    }

    template<typename AsyncReadStream, typename MutableBufferSequence>
    size_t write( AsyncReadStream& s, const MutableBufferSequence& buf, uint64_t timeout_us = -1 ) {
        promise<size_t>::ptr p(new promise<size_t>());
        boost::asio::async_write( s, buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p->wait(timeout_us);
    }

    template<typename AsyncReadStream, typename MutableBufferSequence>
    size_t write_some( AsyncReadStream& s, const MutableBufferSequence& buf, uint64_t timeout_us = -1 ) {
        promise<size_t>::ptr p(new promise<size_t>());
        s.async_write_some(  buf, boost::bind( detail::read_write_handler, p, _1, _2 ) );
        return p->wait(timeout_us);
    }

    namespace tcp {
        typedef boost::asio::ip::tcp::endpoint endpoint;
        typedef boost::asio::ip::tcp::resolver::iterator resolver_iterator;
        typedef boost::asio::ip::tcp::resolver resolver;
        std::vector<endpoint> resolve( const std::string& hostname, const std::string& port, uint64_t timeout_us = -1 );

        template<typename SocketType, typename AcceptorType>
        boost::system::error_code accept( AcceptorType& acc, SocketType& sock, uint64_t timeout_us = -1 ) {
            promise<boost::system::error_code>::ptr p( new promise<boost::system::error_code>() );
            acc.async_accept( sock, boost::bind( detail::error_handler, p, _1 ) );
            return p->wait( timeout_us );
        }

        template<typename AsyncSocket, typename EndpointType>
        boost::system::error_code connect( AsyncSocket& sock, const EndpointType& ep, uint64_t timeout_us = -1 ) {
            promise<boost::system::error_code>::ptr p(new promise<boost::system::error_code>());
            sock.async_connect( ep, boost::bind( detail::error_handler, p, _1 ) );
            return p->wait(timeout_us);
        }


    }
    namespace udp {
        typedef boost::asio::ip::udp::endpoint endpoint;
        typedef boost::asio::ip::udp::resolver::iterator resolver_iterator;
        typedef boost::asio::ip::udp::resolver resolver;
        std::vector<endpoint> resolve( resolver& r, const std::string& hostname, const std::string& port, uint64_t timeout_us = -1 );
    }


} } } // namespace boost::cmt::asio

#endif // _BOOST_CMT_ASIO_HPP_
