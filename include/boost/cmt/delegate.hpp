#ifndef _BOOST_CMT_ASYNC_DELEGATE_HPP_
#define _BOOST_CMT_ASYNC_DELEGATE_HPP_
#include <boost/type_traits/function_traits.hpp>
#include <boost/cmt/detail/async_delegate_impl.hpp>
#include <boost/bind.hpp>

namespace boost { namespace cmt {

    template<typename Signature>
    class async_delegate : 
            public boost::cmt::detail::async_delegate_impl<boost::function_traits<Signature>::arity, Signature >
    {
        public:
            typedef Signature signature;

            template<typename Functor>
            async_delegate( const Functor& slot, thread& s = thread::current() )
            :detail::async_delegate_impl<boost::function_traits<Signature>::arity, Signature >(slot,s){}
    };

} // namespace fl

#endif
