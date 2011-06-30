
include_directories( . )

SET( sources 
                future.cpp
                test.cpp
                thread.cpp
                asio/udp/socket.cpp
             )

add_executable( cmt_test ${sources} )
target_link_libraries( cmt_test boost_context ${Boost_THREAD_LIBRARY} ${Boost_ASIO_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_SIGNALS_LIBRARY})

