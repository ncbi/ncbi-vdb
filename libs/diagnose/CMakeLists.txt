add_library(diagnose STATIC
    diagnose.c
    endpoint.c
)
set_target_properties( diagnose PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

add_definitions (-D__mod__="libs/diagnose")