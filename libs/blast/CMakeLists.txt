set( SRC
    blast-mgr.c
    reader.c
    reference.c
    run-set.c
)

GenerateStaticLibs(vdb-blast "${SRC}" )

add_compile_definitions( __mod__="libs/blast" )