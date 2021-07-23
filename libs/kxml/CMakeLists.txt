if( LibXml2_FOUND  )
    GenerateStaticLibs( kxml
        xml.c
    )

    GenerateStaticLibs( kxfs
        xtoc-parsexml.c
        xtoc.c
    )

    add_compile_definitions (__mod__="libs/kxml")
endif()