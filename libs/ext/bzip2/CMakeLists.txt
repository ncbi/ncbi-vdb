set( SRC
    blocksort
    huffman
    crctable
    randtable
    compress
    decompress
    bzlib
)

GenerateStaticLibs ( bz2 "${SRC}" )

add_compile_definitions( __mod__="libs/ext/bzip2" )