set( SRC
    alignment-writer
    common-reader
    common-writer
    mmarray
    reference-writer
    sequence-writer
)

GenerateStaticLibs( loader "${SRC}" )

add_compile_definitions (__mod__="libs/loader")