set( SRC
    diagnose.c
    endpoint.c
)

GenerateStaticLibs( diagnose "${SRC}" )

ExportStatic( diagnose )
ExportShared( diagnose )

add_compile_definitions( __mod__="libs/diagnose" )

target_link_libraries( diagnose-shared ncbi-vdb )

