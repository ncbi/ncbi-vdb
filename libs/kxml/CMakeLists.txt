if( LibXml2_FOUND  )
    add_library( kxml STATIC
        xml.c
    )

    add_library( kxfs STATIC
        xtoc-parsexml.c
        xtoc.c
    )

    add_compile_definitions (__mod__="libs/kxml")
endif()