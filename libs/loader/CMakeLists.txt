add_library(loader STATIC
    alignment-writer
    common-reader
    common-writer
    mmarray
    reference-writer
    sequence-writer
)

add_definitions (-D__mod__="libs/loader")