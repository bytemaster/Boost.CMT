#ifndef _BOOST_RPC_JSON_CONNECTION_HPP_
#define _BOOST_RPC_JSON_CONNECTION_HPP_
#include <boost/rpc/json.hpp>

namespace boost { namespace rpc { namespace json {
     namespace detail {

        class client_base : public boost::enable_shared_from_this<client_base> {
            public:
                client_base();
                ~client_base();

                uint64_t next_id();
                virtual void invoke( const js::Value& msg, js::Value& rtn );

            private:
                class client_base_private* my;
        };


        struct rpc_functor
        {
            rpc_functor( client& c, const char* name )
            :m_client(c),m_name(name),m_msg(js::Object()),m_obj(m_msg.get_obj())
            {
                 m_obj.push_back( js::Pair( "id", 0 ) );
                 m_obj.push_back( js::Pair( "method", std::string(m_name) ) );
                 m_obj.push_back( js::Pair( "params", js::Array() ) );
            }

            ResultType operator()( const Seq& params ) {
                 obj[0].value_ = m_client.next_id();
                 pack( obj.back().value_, params );
                 ResultType  ret_val;
                 m_client.invoke( m_msg, rtn );
                 unpack( rtn, ret_val );
                 return ret_val;
            }
            client_base& m_client;
            js::Value    m_msg;
            js::Object&  m_obj;
        };


     }  

    namespace tcp {
        typedef boost::asio::ip::tcp::socket socket;
        typedef boost::shared_ptr<socket>    socket_ptr;

        template<typename InterfaceType>
        class client: public boost::reflect::visitor< client<InterfaceType> >, 
                      public boost::reflect::any<InterfaceType>,
                      public detail::client_base
        {
           public:
               typedef boost::shared_ptr<client> ptr;
               client( const socket_ptr& s)
               :m_sock(s)
               { 
                   start_visit(*this); 
                   boost::cmt::async( boost::bind(&client::read_loop,
                                      boost::dynamic_pointer_cast<client>(shared_from_this())) );
               }

               void invoke( const js::Value& msg, js::Value& rtn, uint64_t timeout_us=-1 ) {
                   std::string m =  boost::rpc::json::to_json(msg);
                   stack_retainable<boost::cmt::promise<jsValue> > p;
                   boost::cmt::write( *m_sock, boost::asio::buffer(m.c_str(),m.size()) );
                   m_promises.push_back(&p);
                   rtn = p.wait(timeout_us);
                   m_promises.remove(&p);
               }

               template<typename InterfaceName, typename M>
               bool accept( M& m, const char* name ) {
                    m.m_delegate = rpc_functor<typename M::fused_params, typename M::result_type>(*this,name);
                    return true;
               }
           private:
                void read_loop() {
                    
                }

                socket_ptr                           m_sock;
                std::list<boost::cmt::promise_base*> m_promises;
        };
    } // namespace tcp

    namespace udp {


    } // namespace udp

} } } // boost::rpc::json

#endif

