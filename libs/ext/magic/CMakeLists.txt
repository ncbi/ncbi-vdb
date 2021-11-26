# libmagic, version 5.04 (ftp://ftp.astron.com/pub/file/file-5.04.tar.gz) configured for 64 bit Linux
if ( ${OS} STREQUAL "linux" )
    add_library ( magic-static STATIC
        magic
        apprentice
        softmagic
        ascmagic
        encoding
        compress
        is_tar
        readelf
        print
        fsmagic
        funcs
        apptype
        cdf
        cdf_time
        readcdf
        strlcat
        strlcpy
    )
    ExportStatic( magic-static true )
    add_compile_definitions( HAVE_CONFIG_H=1 )
    add_compile_definitions( __mod__="libs/ext/magic" )
    include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )
endif()
