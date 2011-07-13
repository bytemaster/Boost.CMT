
include_directories( include )

SET( sources 
            src/usclock.cpp
            src/future.cpp
            src/thread.cpp
            src/asio/acceptor.cpp
            src/asio/udp/socket.cpp
             )

SETUP_LIBRARY( boost_cmt SOURCES ${sources}
                             LIBRARIES 
                             LIBRARY_TYPE STATIC )

add_subdirectory( examples )
