add_library(vdb-sqlite STATIC
    sqlite3.c
    sqlite3vdb.c
)
set_target_properties( vdb-sqlite PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

add_definitions (-D__mod__="libs/vdb-sqlite")
