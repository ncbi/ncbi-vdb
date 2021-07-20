set( LIBZ_SRC
    adler32
    compress
    crc32
    deflate
    gzclose
    gzlib
    gzread
    gzwrite
    infback
    inffast
    inflate
    inftrees
    trees
    uncompr
    zutil
)

add_library( z STATIC ${LIBZ_SRC} )

add_compile_definitions( __mod__="libs/ext/zlib" )
