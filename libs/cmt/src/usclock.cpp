#include <boost/cmt/usclock.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread_time.hpp>
#ifdef WIN32
#include <winsock2.h>

namespace boost { namespace cmt {

    static LARGE_INTEGER            ticks_per_second;
    static LARGE_INTEGER            start_tick;
    static boost::posix_time::ptime start_time;
	uint64_t						start_us;
    uint64_t usclock_current();
    uint64_t usclock_init()
    {
        QueryPerformanceFrequency( &ticks_per_second );
        QueryPerformanceCounter( &start_tick );
        start_time = boost::posix_time::microsec_clock::universal_time();
		start_us = usclock_current();
		return start_us;
    }

    uint64_t usclock_current()
    {
	   static uint64_t last_time = usclock_init();

        LARGE_INTEGER current_tick;
        LARGE_INTEGER cputime;
        QueryPerformanceCounter(&current_tick);
        cputime.QuadPart = current_tick.QuadPart - start_tick.QuadPart;
        uint64_t        newtime  = uint64_t((double)cputime.QuadPart/(double)ticks_per_second.QuadPart * 1000 * 1000);
        
        return last_time =  newtime > last_time ? newtime : last_time + 1;
    }

    /**
     *  @return the number of microseconds from the first time this method was called
     */
    uint64_t usclock()
    {
        return usclock_current() - int64_t(start_us);
	}
	uint64_t utc_clock()
	{
		static boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
		static uint64_t start_utc = (boost::posix_time::microsec_clock::universal_time() - epoch).total_microseconds();
		static uint64_t start_us = usclock();
		static uint64_t offset   = start_utc - start_us;

		return offset + usclock();
	}
	uint64_t sync_utc_clock()
	{
		static boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
		uint64_t start_utc = (boost::posix_time::microsec_clock::universal_time() - epoch).total_microseconds();

		return start_utc;
	}
} } // namespace boost::cmt




#else
#include <sys/time.h>
namespace boost { namespace cmt {

timeval start_timeval;
uint64_t start_time;

uint64_t usclock_init()
{
    gettimeofday(&start_timeval,NULL);
    return start_time = uint64_t(start_timeval.tv_sec) * 1000000 + start_timeval.tv_usec;
}
uint64_t usclock_current()
{
    static uint64_t last_time = usclock_init();
    timeval cur_timeval;
    gettimeofday(&cur_timeval,NULL);
    uint64_t new_time = uint64_t(cur_timeval.tv_sec) * uint64_t(1000000) + cur_timeval.tv_usec;

           //     dout( debug, "new_time: %1%   sec: %2% tv_usec: %3%", %  new_time % cur_timeval.tv_sec % cur_timeval.tv_usec );
    //return last_time = new_time > last_time ? new_time : last_time + 1;
    return last_time = new_time > last_time ? new_time : last_time + 1;
}


/**
 *  @return the number of microseconds from the first time this method was called
 */
uint64_t usclock()
{
    return usclock_current() - start_time;
}

uint64_t utc_clock()
{
    static boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
    static uint64_t start_utc = (boost::posix_time::microsec_clock::universal_time() - epoch).total_microseconds();
    static uint64_t start_us = usclock();
    static uint64_t offset   = start_utc - start_us;

    return offset + usclock();
}
uint64_t sys_clock()
{
    static boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
    static uint64_t start_utc = (boost::get_system_time() - epoch).total_microseconds();
    static uint64_t start_us = usclock();
    static uint64_t offset   = start_utc - start_us;

    if( (usclock() / 1000000) % 10 == 0 ) {
        start_utc = (boost::get_system_time() - epoch).total_microseconds();
        start_us = usclock();
        offset   = start_utc - start_us;
    }

    return offset + usclock();
}

boost::posix_time::ptime to_ptime( uint64_t t ) {
    static boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
    return epoch + boost::posix_time::seconds(t/1000000) + boost::posix_time::microseconds(t%1000000);
}

uint64_t sync_utc_clock()
{
    static boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
    uint64_t start_utc = (boost::posix_time::microsec_clock::universal_time() - epoch).total_microseconds();

    return start_utc;
}


} } // namespace boost::cmt

#endif




