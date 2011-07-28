
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

SET( headers 
            include/boost/cmt/asio.hpp
            include/boost/cmt/delegate.hpp
            include/boost/cmt/error.hpp
            include/boost/cmt/future.hpp
            include/boost/cmt/mutex.hpp
            include/boost/cmt/retainable.hpp
            include/boost/cmt/signals.hpp
            include/boost/cmt/task.hpp
            include/boost/cmt/thread.hpp
            include/boost/cmt/usclock.hpp
            include/boost/cmt/detail/async_delegate_impl.hpp
            )

SETUP_LIBRARY( boost_cmt SOURCES ${sources}
                             LIBRARIES 
                             LIBRARY_TYPE STATIC )

install( DIRECTORY include/boost/cmt DESTINATION include/boost )
add_subdirectory( examples )
