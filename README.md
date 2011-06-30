Boost.CMT - Cooperative Multitasking Library
---------------------------------------

This library builds on top of Boost.Context to provide 
an effecient coopertive multitasking API with a focus 
on ease of use and staying out of your way.

### Notice ###

    This library is not part of the official Boost C++ library, but
    is written, to the best of my ability, to follow the best practices
    established by the Boost community and with the hope of being 
    considered for inclusion with a future Boost release.

### Requirements ###
    
    This code has been tested on Mac OS X using gcc 4.5.0 and boost 4.6.1.
    Boost.Context is included with local modifications to build and run on
    Mac OS X with ucontext support.  (fcontext support causes floating point
    exceptions)


### Basic Example ###

    int hello(const std::string& world ) {
        return world.size(); 
    }

    void bench() {
        ptime start = microsec_clock::universal_time();
        int sum = 0;
        for( uint32_t i = 0; i < 1000; ++i ) 
            sum += async<int>( boost::bind(hello, "world"), "hello_func" ).wait();
        ptime end = microsec_clock::universal_time();
        slog( "%1% calls/sec", (1000.0/((stop-start).total_microseconds()/1000000.0)) );
    }

    int main( int argc, char** argv ) {
        async( bench );
        boost::cmt::exec(); 
    }

### Signal Example ###

    boost::signal<void(std::string)> test_signal;
   
    void delay()
    {
        boost::cmt::usleep(2000000);
        test_signal("hello world!");
    }

    void wait_on_signal() {
        std::string rtn = boost::cmt::wait<std::string>(test_signal);
    }

    int main( int argc, char** argv ) {
         async( delay );
         async( wait_on_signal );
         boost::cmt::exec(); 
    }


