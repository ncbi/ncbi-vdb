add_library(vdb-sqlite STATIC
    sqlite3.c
    sqlite3vdb.c
)

add_definitions (-D__mod__="libs/vdb-sqlite")
