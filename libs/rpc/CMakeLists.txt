
include_directories( include )
include_directories( ../cmt/include )

SET( sources 
#            src/client_base.cpp
            src/json/json_spirit_reader.cpp
            src/json/json_spirit_value.cpp
            src/json/json_spirit_writer.cpp
             )


add_executable( jtest examples/json.cpp  )
target_link_libraries( jtest boost_cmt
                        boost_context 
                        boost_rpc 
                       ${Boost_THREAD_LIBRARY} 
                       ${Boost_SERIALIZATION_LIBRARY} 
                       ${Boost_ASIO_LIBRARY} 
                       ${Boost_SYSTEM_LIBRARY} 
                       ${Boost_SIGNALS_LIBRARY})


SETUP_LIBRARY( boost_rpc SOURCES ${sources}
                             LIBRARIES 
                             LIBRARY_TYPE STATIC )

#add_executable( crpc examples/rpc_client.cpp  )
#target_link_libraries( crpc boost_rpc boost_cmt
#                        boost_context 
#                       ${Boost_THREAD_LIBRARY} 
#                       ${Boost_SERIALIZATION_LIBRARY} 
#                       ${Boost_ASIO_LIBRARY} 
#                       ${Boost_SYSTEM_LIBRARY} 
#                       ${Boost_SIGNALS_LIBRARY})

