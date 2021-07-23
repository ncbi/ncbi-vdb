GenerateStaticLibs( diagnose
    diagnose.c
    endpoint.c
)

ExportStatic( diagnose )
ExportShared( diagnose )

add_compile_definitions( __mod__="libs/diagnose" )

target_link_libraries( diagnose-shared ncbi-vdb )

