if( LibXml2_FOUND )
    set( SRC_XML xml.c )
    GenerateStaticLibs( kxml "${SRC_XML}" )

    set( SRC_KXFS xtoc-parsexml.c xtoc.c )
    GenerateStaticLibs( kxfs "${SRC_KXFS}" )

    add_compile_definitions (__mod__="libs/kxml")
endif()