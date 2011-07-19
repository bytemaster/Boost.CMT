include_directories( include )
include_directories( ../move/include )

IF( APPLE )
SET_PROPERTY( SOURCE 
              src/asm/fcontext_x86_64_sysv_macho_gas.S 
              PROPERTY LANGUAGE C)
SET( sources src/protected_stack_posix.cpp 
             src/stack_helper_posix.cpp 
             src/asm/fcontext_x86_64_sysv_macho_gas.S 
             )
ELSE (APPLE)

ENABLE_LANGUAGE(ASM_MASM) 

SET( sources src/protected_stack_windows.cpp 
             src/stack_helper_windows.cpp 
             src/seh.cpp 
             src/asm/fcontext_i386_ms_pe_masm.asm
             )
ENDIF(APPLE)

SETUP_LIBRARY( boost_context SOURCES ${sources}
                             LIBRARIES 
                             LIBRARY_TYPE STATIC )

ADD_SUBDIRECTORY( example )
