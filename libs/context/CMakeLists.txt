include_directories( include )
include_directories( ../move/include )

SET_PROPERTY( SOURCE 
              src/asm/fcontext_x86_64_sysv_macho_gas.S 
              PROPERTY LANGUAGE C)
SET( sources src/protected_stack_posix.cpp 
             src/stack_helper_posix.cpp 
             src/asm/fcontext_x86_64_sysv_macho_gas.S 
             )
SETUP_LIBRARY( boost_context SOURCES ${sources}
                             LIBRARIES 
                             LIBRARY_TYPE STATIC )

ADD_SUBDIRECTORY( example )
