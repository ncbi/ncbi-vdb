set( SRC
    diagnose.c
    endpoint.c
)

add_library( diagnose STATIC ${SRC} )
set_target_properties( diagnose PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

add_library( diagnose-shared SHARED ${SRC} )

set_target_properties( diagnose-shared PROPERTIES OUTPUT_NAME diagnose )

add_definitions (-D__mod__="libs/diagnose")