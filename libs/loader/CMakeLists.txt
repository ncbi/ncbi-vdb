add_library(loader STATIC
    alignment-writer.c
    common-reader.c
    common-writer.c
    mmarray.c
    reference-writer.c
    sequence-writer.c
)

add_definitions (-D__mod__="libs/loader")