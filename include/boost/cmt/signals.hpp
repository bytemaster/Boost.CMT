#ifndef _BOOST_CMT_SIGNALS_HPP
#define _BOOST_CMT_SIGNALS_HPP
#include <boost/signal.hpp>
#include <boost/cmt/task.hpp>

namespace boost { namespace cmt {
   template<typename T>
   T wait( boost::signal<void(T)>& sig, const microseconds& timeout_us=microseconds::max() ) {
       typename promise<T>::ptr p(new promise<T>());
       boost::signals::scoped_connection c = sig.connect( boost::bind(&promise<T>::set_value,p,_1) );
       return p->wait( timeout_us ); 
   }

   void wait( boost::signal<void()>& sig, const microseconds& timeout_us=microseconds::max() ) {
       promise<void_t>::ptr p(new promise<void_t>());
       boost::signals::scoped_connection c = sig.connect( boost::bind(&promise<void_t>::set_value,p,void_t()) );
       p->wait( timeout_us ); 
   }
} }

#endif
