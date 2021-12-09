set( SRC
    diagnose.c
    endpoint.c
)

add_compile_definitions( __mod__="libs/diagnose" )

GenerateStaticLibs( diagnose "${SRC}" )
ExportStatic( diagnose false )

if( WIN32 )
    set( extra_libs ncbi-vdb Ws2_32 Crypt32 )
else()
    set( extra_libs ncbi-vdb )
endif()
ExportShared( diagnose false "${extra_libs}")
