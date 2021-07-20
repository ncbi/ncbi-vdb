add_library(vdb-sqlite STATIC
    sqlite3.c
    sqlite3vdb.c
)

ExportStatic( vdb-sqlite )
ExportShared( vdb-sqlite )

add_compile_definitions (__mod__="libs/vdb-sqlite")

target_link_libraries( vdb-sqlite-shared ncbi-vdb ncbi-ngs )
