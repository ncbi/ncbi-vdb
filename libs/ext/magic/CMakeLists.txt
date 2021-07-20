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

add_compile_definitions( HAVE_CONFIG_H=1 )
add_compile_definitions( __mod__="libs/ext/magic" )
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )