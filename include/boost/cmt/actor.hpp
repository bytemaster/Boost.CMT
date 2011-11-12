#ifndef _CMT_ACTOR_HPP_
#define _CMT_ACTOR_HPP_
#include <boost/cmt/thread.hpp>
#include <boost/reflect/any_ptr.hpp>
#include <boost/cmt/actor_interface.hpp>

namespace boost { namespace cmt {

  /**
   *  An actor is assigned to a specific thread, and if a method is called on the actor
   *  from another thread then it is posted asynchronously to the proper thread.
   *
   *  All methods return future<R>.
   *
   */
  template<typename InterfaceType>
  class actor : public boost::reflect::any_ptr<InterfaceType, boost::cmt::actor_interface>, public detail::actor_base {
    public:
      typedef boost::shared_ptr<actor>   ptr;

      actor(boost::cmt::thread* t = boost::cmt::thread::current() )
      :actor_base(t){
      }
      template<typename T>
      actor( T* v, boost::cmt::thread* t = boost::cmt::thread::current() )  
      :actor_base(t) {
        this->m_ptr = v;
        cmt::actor_interface::set_vtable(*this->m_vtable,*v,this);
      }
      template<typename T>
      actor( const boost::shared_ptr<T>& v,boost::cmt::thread* t = boost::cmt::thread::current() ) 
      :actor_base(t) {
        this->m_ptr = v;
        cmt::actor_interface::set_vtable(*this->m_vtable,*v,this);
      }
      template<typename OtherInterface,typename OtherDelegate>
      actor( const boost::reflect::any_ptr<OtherInterface,OtherDelegate>& v,
             boost::cmt::thread* t = boost::cmt::thread::current() ) 
      :actor_base(t) {
        this->m_ptr = v;
        cmt::actor_interface::set_vtable( *this->m_vtable, 
                          *boost::any_cast<boost::reflect::any_ptr<OtherInterface,OtherDelegate>&>(this->m_ptr), this );
      }
  };

} } // namespace boost

#endif

