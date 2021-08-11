set( SRC
    diagnose.c
    endpoint.c
)

GenerateStaticLibs( diagnose "${SRC}" )

ExportStatic( diagnose false )
ExportShared( diagnose false )

add_compile_definitions( __mod__="libs/diagnose" )

if( WIN32 )
    target_link_libraries( diagnose-shared ncbi-vdb Ws2_32 Crypt32 )
    target_link_libraries( diagnose-shared-md ncbi-vdb Ws2_32 Crypt32 )
else()
#add_dependencies( diagnose-shared ncbi-vdb )
    target_link_libraries( diagnose-shared ncbi-vdb )
endif()
