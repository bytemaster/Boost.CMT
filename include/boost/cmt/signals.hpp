/**
 *  @file boost/cmt/signals.hpp
 *  @brief Provides integration with Boost.Signals
 */
#ifndef _BOOST_CMT_SIGNALS_HPP
#define _BOOST_CMT_SIGNALS_HPP
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <boost/cmt/task.hpp>
#include <boost/cmt/thread.hpp>

namespace boost { namespace cmt {
   /**
    * @brief Wait on a signal with one parameter which will be returned when the signal is emitted.
    *
    * @todo create a version that will wait on signals with multiple parameters and return
    * a <code>boost::fusion::vector<></code> of results.
    */
   template<typename T>
   inline T wait( boost::signal<void(T)>& sig, const microseconds& timeout_us=microseconds::max() ) {
           typename promise<T>::ptr p(new promise<T>());
           boost::signals::scoped_connection c = sig.connect( boost::bind(&promise<T>::set_value,p,_1) );
           return p->wait( timeout_us ); 
   }

   /**
    * @brief Wait on a signal with no parameters.
    */
   inline void wait( boost::signal<void()>& sig, const microseconds& timeout_us=microseconds::max() ) {
           promise<void_t>::ptr p(new promise<void_t>());
           boost::signals::scoped_connection c = sig.connect( boost::bind(&promise<void_t>::set_value,p,void_t()) );
           p->wait( timeout_us ); 
   }
} }

#endif
