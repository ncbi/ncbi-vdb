add_library(diagnose STATIC
    diagnose.c
    endpoint.c
)

add_definitions (-D__mod__="libs/diagnose")