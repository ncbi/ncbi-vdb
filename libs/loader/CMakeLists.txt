add_library(loader STATIC
    alignment-writer
    common-reader
    common-writer
    mmarray
    reference-writer
    sequence-writer
)

add_compile_definitions (__mod__="libs/loader")