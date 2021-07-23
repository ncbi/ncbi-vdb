GenerateStaticLibs(vdb-blast
    blast-mgr.c
    reader.c
    reference.c
    run-set.c
)

add_compile_definitions( __mod__="libs/blast" )