#ifndef _BOOST_CMT_USCLOCK_HPP_
#define _BOOST_CMT_USCLOCK_HPP_
#include <stdint.h>
#include <boost/date_time/posix_time/ptime.hpp>

namespace boost { namespace cmt {

uint64_t                 usclock();
uint64_t                 utc_clock();
uint64_t                 sys_clock();
uint64_t                 sync_utc_clock();
boost::posix_time::ptime to_ptime( uint64_t t );

} } // boost::cmt

#endif
