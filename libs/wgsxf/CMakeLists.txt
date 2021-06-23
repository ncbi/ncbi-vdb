add_library(wgsxf STATIC
    build_read_type.c
    build_scaffold.c
)

add_definitions (-D__mod__="libs/wgsxf")