include_directories( include )

SET( sources 
            src/json_spirit_reader.cpp
            src/json_spirit_value.cpp
            src/json_spirit_writer.cpp
             )

SETUP_LIBRARY( json_spirit SOURCES ${sources}
                             LIBRARIES 
                             LIBRARY_TYPE STATIC )

