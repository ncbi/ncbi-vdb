add_library(vdb-blast STATIC
    blast-mgr.c
    reader.c
    reference.c
    run-set.c
)

add_definitions (-D__mod__="libs/blast")