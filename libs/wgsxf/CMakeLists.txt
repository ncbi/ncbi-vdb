set( WGSXF_SRC
    build_scaffold
    build_read_type
)


GenerateStaticLibs( wgsxf "${WGSXF_SRC}" )
GenerateStaticLibs( wwgsxf "${WGSXF_SRC}" )

add_compile_definitions (__mod__="libs/wgsxf")