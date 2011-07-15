#ifndef _BOOST_RPC_JSON_CLIENT_HPP_
#define _BOOST_RPC_JSON_CLIENT_HPP_
#include <boost/rpc/json.hpp>

namespace boost { namespace rpc { namespace json {
     namespace detail {

        class client_base : public boost::enable_shared_from_this<client_base> {
            public:
                client_base();
                ~client_base();

                uint64_t next_id();
                virtual void invoke( const js::Value& msg, js::Value& rtn_msg );

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
                 js::Value  rtn_msg;
                 m_client.invoke( m_msg, rtn_msg );
                 ResultType  ret_val;
                 if( value_contains( ret_msg, "result" ) )  {
                     unpack( value_get(ret_msg,"result"), ret_val );
                     return ret_val;
                 }
                 if( value_contains( ret_msg, "error" ) ) {
                    error_object e;
                    unpack( value_get(ret_msg,"result"), e );
                    BOOST_THROW_EXCEPTION( e );
                 }
            }
            client_base& m_client;
            js::Value    m_msg;
            js::Object&  m_obj;
        };


     }  

    namespace tcp {
        typedef boost::cmt::asio::tcp::iostream::ptr sock_ios_ptr;

        template<typename InterfaceType>
        class client: public boost::reflect::visitor< client<InterfaceType> >, 
                      public boost::reflect::any<InterfaceType>,
                      public detail::client_base
        {
           public:
               typedef boost::shared_ptr<client> ptr;
               client( const socket_ios_ptr& s)
               :m_sock_ios(s)
               { 
                   start_visit(*this); 
                   boost::cmt::async( boost::bind(&client::read_loop,
                                      boost::dynamic_pointer_cast<client>(shared_from_this())) );
               }

               void invoke( const js::Value& msg, js::Value& rtn, uint64_t timeout_us=-1 ) {
                   stack_retainable<boost::cmt::promise<js::Value> > p;
                   boost::rpc::json::to_json(m_sock, msg);
                   int id = msg["id"].get_int();
                   m_promises[id] = &p;
                   rtn = p.wait(timeout_us);
                   m_promises.erase(id);
               }

               template<typename InterfaceName, typename M>
               bool accept( M& m, const char* name ) {
                    m.m_delegate = rpc_functor<typename M::fused_params, typename M::result_type>(*this,name);
                    return true;
               }
           private:
                void read_loop() {
                    js::Stream_reader<boost::cmt::asio::tcp::iostream, js::Value> reader(*m_sock_ios);
                    js::Value v;
                    message m;
                    while( reader.read_next(v) ) {
                        boost::rpc::json::unapck( v, m ); 
                        int id = *m.id;
                        std::map<int,boost::cmt::promise_base*>::iterator itr = m_promises.find(id);
                        if( itr != m_promises.end() ) {
                            itr->second->set_value(v);
                            m_promises.erase(itr);
                        } else {
                           elog( "Unexpected value: %1%", json_spirit::write_formated(v) );
                        }
                    }
                }

                std::map<int,boost::cmt::promise_base*> m_promises;
        };
    } // namespace tcp

    namespace udp {


    } // namespace udp

} } } // boost::rpc::json

#endif

