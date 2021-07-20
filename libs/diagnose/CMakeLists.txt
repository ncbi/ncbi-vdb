add_library( diagnose STATIC
    diagnose.c
    endpoint.c
)

ExportStatic( diagnose )
ExportShared( diagnose )

add_compile_definitions( __mod__="libs/diagnose" )