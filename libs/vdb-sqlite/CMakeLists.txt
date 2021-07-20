add_library(vdb-sqlite STATIC
    sqlite3.c
    sqlite3vdb.c
)

ExportStatic( vdb-sqlite )
ExportShared( vdb-sqlite )

add_dependencies( vdb-sqlite-shared ncbi-vdb )

add_compile_definitions (__mod__="libs/vdb-sqlite")
