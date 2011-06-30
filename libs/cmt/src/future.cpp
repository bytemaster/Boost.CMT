#include <boost/cmt/future.hpp>
#include <boost/cmt/thread.hpp>

namespace boost { namespace cmt {

    void promise_base::enqueue_thread() {
        m_blocked_thread =&thread::current();
    }
    void promise_base::wait( uint64_t timeout_us ) {
        thread::current().wait( shared_from_this(), timeout_us ); 
    }

    void promise_base::notify() {
        if( m_blocked_thread ) m_blocked_thread->notify(shared_from_this());
    }

} } // namespace boost::cmt
