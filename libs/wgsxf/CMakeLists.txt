set( WGSXF_SRC
    build_scaffold
    build_read_type
)


add_library( wgsxf STATIC ${WGSXF_SRC} )
add_library( wwgsxf STATIC ${WGSXF_SRC} )

add_compile_definitions (__mod__="libs/wgsxf")