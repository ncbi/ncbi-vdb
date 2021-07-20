add_library ( bz2 STATIC
    blocksort
    huffman
    crctable
    randtable
    compress
    decompress
    bzlib
)

add_compile_definitions( __mod__="libs/ext/bzip2" )