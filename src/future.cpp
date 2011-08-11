#include <boost/cmt/future.hpp>
#include <boost/cmt/thread.hpp>

namespace boost { namespace cmt {

    void promise_base::enqueue_thread() {
        m_blocked_thread =&thread::current();
    }
    void promise_base::wait( const boost::chrono::microseconds& timeout_us ) {
        thread::current().wait( ptr(this,true), timeout_us ); 
    }

    void promise_base::notify() {
        if( m_blocked_thread ) m_blocked_thread->notify(ptr(this,true));
    }

} } // namespace boost::cmt
