set( WGSXF_SRC
    build_scaffold
    build_read_type
)


add_library( wgsxf STATIC ${WGSXF_SRC} )
add_library( wwgsxf STATIC ${WGSXF_SRC} )

add_definitions (-D__mod__="libs/wgsxf")