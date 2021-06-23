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

add_definitions ( -D__mod__="libs/ext/magic" )