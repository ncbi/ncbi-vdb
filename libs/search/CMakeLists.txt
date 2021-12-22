# some modules produce a ton of output with DEBUG defined
remove_definitions(-DDEBUG)

set( SRC
    agrep-dp.c
    agrep-myers.c
    agrep-myersunltd.c
    agrep-wumanber.c
    fgrep-aho.c
    fgrep-boyermoore.c
    fgrep-dumb.c
    nucstrstr.c
    search.c
    sw-search.c
)

GenerateStaticLibs( ksrch "${SRC}" )

add_compile_definitions (__mod__="libs/search")
