set( SRC
    diagnose.c
    endpoint.c
)

add_compile_definitions( __mod__="libs/diagnose" )

GenerateStaticLibs( diagnose "${SRC}" )
ExportStatic( diagnose false )

set( extra_libs ncbi-vdb )
if( WIN32 )
    set( extra_libs "${extra_libs} Ws2_32 Crypt32" )
endif()
ExportShared( diagnose false ${extra_libs})


