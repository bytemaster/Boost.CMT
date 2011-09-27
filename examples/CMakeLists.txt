

SET( libraries 
    #/usr/local/lib/tcmalloc/libtcmalloc.a
    boost_cmt
    boost_context 
    ${Boost_THREAD_LIBRARY} 
    ${Boost_CHRONO_LIBRARY} 
    ${Boost_ASIO_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_SIGNALS_LIBRARY})
    

#add_executable( tcp_ios  tcp_iostream.cpp  )
add_executable( cmt_test test.cpp  )
#add_executable( thread_test thread_test.cpp  )


#target_link_libraries( tcp_ios ${libraries} )
target_link_libraries( cmt_test ${libraries} )
#target_link_libraries( thread_test ${libraries} )

