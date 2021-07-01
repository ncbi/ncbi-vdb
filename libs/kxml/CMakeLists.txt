if( LibXml2_FOUND  )
    add_library( kxml STATIC
        xml.c
    )

    add_library( kxfs STATIC
        xtoc-parsexml.c
        xtoc.c
    )

    add_definitions (-D__mod__="libs/kxml")
endif()