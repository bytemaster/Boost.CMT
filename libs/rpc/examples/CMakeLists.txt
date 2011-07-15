SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    boost_cmt
    boost_context 
    ${Boost_THREAD_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})

add_executable( rpcs  rpc_server.cpp  )
target_link_libraries( rpcs ${libraries} )
