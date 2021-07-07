add_library( diagnose STATIC
    diagnose.c
    endpoint.c
)

ExportStatic( diagnose )
ExportShared( diagnose )

add_definitions (-D__mod__="libs/diagnose")