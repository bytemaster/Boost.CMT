
include_directories( include )
include_directories( ../context/include )
include_directories( ../move/include )
include_directories( ../atomic/include )

SET( sources 
            src/usclock.cpp
            src/future.cpp
            src/thread.cpp
            src/asio.cpp
            src/asio/acceptor.cpp
            src/asio/udp/socket.cpp
            src/asio/tcp/socket.cpp
             )


SETUP_LIBRARY( boost_cmt SOURCES ${sources}
                             LIBRARIES 
                             LIBRARY_TYPE STATIC )

install( DIRECTORY include/boost/cmt DESTINATION include/boost )
add_subdirectory( examples )
