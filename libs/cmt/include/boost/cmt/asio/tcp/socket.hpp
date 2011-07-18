#ifndef _BOOST_CMT_ASIO_TCP_SOCKET_HPP
#define _BOOST_CMT_ASIO_TCP_SOCKET_HPP
#include <boost/asio.hpp>

namespace boost { namespace cmt { namespace asio { namespace tcp {

    /**
     *  Provides a buffered socket based on boost::asio.  
     */
    class socket  : public boost::asio::ip::tcp::socket {
        public:
            typedef boost::shared_ptr<socket> ptr;

            socket();
            ~socket();

            boost::system::error_code connect( const boost::asio::ip::tcp::endpoint& ep );

            /**
             * Reads one element at a time.
             */
            struct iterator : public std::iterator<std::input_iterator_tag,char,void> {
                iterator( boost::cmt::asio::tcp::socket* _s = NULL)
                :s(_s){ if(_s){++*this;}  }

                inline const char& operator*()const  { return value;  }
                inline const char* operator->()const { return &value; }
                inline char& operator*() { return value;  }
                inline char* operator->(){ return &value; }

                iterator& operator++();
                iterator operator++(int);

                bool operator == ( const iterator& i )const { return s == i.s; }
                bool operator != ( const iterator& i )const { return s != i.s; }

                private:
                    char                           value;
                    boost::cmt::asio::tcp::socket* s;
            };

            size_t read_some( char* buffer, size_t size );
            size_t read( char* buffer, size_t size );
            size_t try_read( char* buffer, size_t size );
            size_t write( const char* buffer, size_t size );

        private:
            void  write_loop( uint8_t write_buf_idx );
            std::vector<char>  read_buf;
            std::vector<char>  write_buf[2];
            std::vector<char>* cur_write_buf;

            uint8_t           cur_wbuf_idx;
            size_t            read_pos;
            size_t            last_avail;
    };

} } } } // namespace boost::cmt::asio::tcp

#endif
