add_library ( magic STATIC
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

add_definitions (-DHAVE_CONFIG_H=1)
add_definitions ( -D__mod__="libs/ext/magic" )
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )