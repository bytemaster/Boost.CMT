
include_directories( include )

SET( sources 
            src/future.cpp
            src/thread.cpp
            src/asio/udp/socket.cpp
             )

SETUP_LIBRARY( boost_cmt SOURCES ${sources}
                             LIBRARIES 
                             LIBRARY_TYPE STATIC )

add_executable( cmt_test examples/test.cpp  )
target_link_libraries( cmt_test boost_cmt
                        boost_context 
                       ${Boost_THREAD_LIBRARY} 
                       ${Boost_ASIO_LIBRARY} 
                       ${Boost_SYSTEM_LIBRARY} 
                       ${Boost_SIGNALS_LIBRARY})

