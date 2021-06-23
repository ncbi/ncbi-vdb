add_library ( bz2 STATIC
    blocksort
    huffman
    crctable
    randtable
    compress
    decompress
    bzlib
)

add_definitions ( -D__mod__="libs/ext/bzip2" )