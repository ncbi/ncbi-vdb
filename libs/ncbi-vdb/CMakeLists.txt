# all static libraries in this directory are published
if (CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
elseif( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG})
elseif( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG})
endif()

if( LibXml2_FOUND  )
    set( KXML "$<TARGET_OBJECTS:kxml>")
    set( LIBXML2 "xml2")
else()
    set( KXML "")
    set( LIBXML2 "")
endif()

set( VDB_LIB_CMN_TARGETS
    align-access
    ncbi-bam
    vfs
    cloud
    mbedx509
    mbedtls
    mbedcrypto
    kns
    kfg
    krypto
    ksrch
    kfs
    klib
    judy
    kproc
    kq
    kfc
    bz2
    z
    zstd
)

if( LibXml2_FOUND  )
    #set( KXML "$<TARGET_OBJECTS:kxml>")
    list( APPEND VDB_LIB_CMN_TARGETS "kxml" )
    set( LIBXML2 "xml2")
else()
    #set( KXML "")
    set( LIBXML2 "")
endif()

set( VDB_LIB_CMN "" )
foreach( target ${VDB_LIB_CMN_TARGETS})
    list( APPEND VDB_LIB_CMN "$<TARGET_OBJECTS:${target}>" )
endforeach()

set( VDB_LIB_RD_TARGETS
    ${VDB_LIB_CMN_TARGETS}
    align-reader
    ascp
    srareader
    sradb
    vdb-blast
    vdb
    kdb
    axf
    sraxf
    wgsxf
    vxf
)

set( VDB_LIB_RD "" )
foreach( target ${VDB_LIB_RD_TARGETS})
    list( APPEND VDB_LIB_RD "$<TARGET_OBJECTS:${target}>" )
endforeach()

set( VDB_LIB_RDWR_TARGETS
    ${VDB_LIB_CMN_TARGETS}
    align-writer
    wsradb
    wvdb
    wkdb
    waxf
    wsraxf
    wwgsxf
    wvxf
)

set( VDB_LIB_RDWR "" )
foreach( target ${VDB_LIB_RDWR_TARGETS})
    list( APPEND VDB_LIB_RDWR "$<TARGET_OBJECTS:${target}>" )
endforeach()

if( WIN32 )
    add_library( ncbi-ngs STATIC
        $<TARGET_OBJECTS:ngs>
        $<TARGET_OBJECTS:ngs-jni>
        $<TARGET_OBJECTS:ngs-py>
    )
    MSVS_StaticRuntime( ncbi-ngs )
    ExportStatic( ncbi-ngs )

    add_library( ncbi-ngs-md STATIC
        $<TARGET_OBJECTS:ngs-md>
        $<TARGET_OBJECTS:ngs-jni-md>
        $<TARGET_OBJECTS:ngs-py-md>
    )
    MSVS_DLLRuntime( ncbi-ngs-md )
    ExportStatic( ncbi-ngs-md )
else()
    add_library( ncbi-ngs STATIC
        $<TARGET_OBJECTS:ngs>
        $<TARGET_OBJECTS:ngs-jni>
        $<TARGET_OBJECTS:ngs-py>
    )
    ExportStatic( ncbi-ngs )
endif()

# set( NCBI_NGS_CPP ncbi-ngs-c++)
# add_library( ${NCBI_NGS_CPP}-exported STATIC
    # $<TARGET_OBJECTS:ncbi-ngs-c++>
# )
#ExportStatic( ncbi-ngs-c++ )
# the output goes to .../lib
# set_target_properties( ${NCBI_NGS_CPP}-exported PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
# # make the output name versioned, create all symlinks
# add_custom_command(TARGET ${NCBI_NGS_CPP}-exported
    # POST_BUILD
    # COMMAND rm -f lib${NCBI_NGS_CPP}.a.${VERSION}
    # COMMAND mv ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${NCBI_NGS_CPP}-exported.a lib${NCBI_NGS_CPP}.a.${VERSION}
    # COMMAND ln -f -s lib${NCBI_NGS_CPP}.a.${VERSION} lib${NCBI_NGS_CPP}.a.${MAJVERS}
    # COMMAND ln -f -s lib${NCBI_NGS_CPP}.a.${MAJVERS} lib${NCBI_NGS_CPP}.a
    # COMMAND ln -f -s lib${NCBI_NGS_CPP}.a lib${NCBI_NGS_CPP}-static.a
    # WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
# )


add_library( ncbi-vdb STATIC "${VDB_LIB_RD}" )
ExportStatic( ncbi-vdb )
MSVS_StaticRuntime( ncbi-vdb )

# here, creating the shared version is a bit trickier than just calling ExportShared
set( VDB_LIB_RD_SHARED
    ${VDB_LIB_RD}
    $<TARGET_OBJECTS:ngs>
    $<TARGET_OBJECTS:ngs-jni>
    $<TARGET_OBJECTS:ngs-py>
)

if(WIN32)
    add_library( ncbi-vdb-shared SHARED "${VDB_LIB_RD_SHARED}" )
#    set_target_properties( ncbi-vdb-shared PROPERTIES OUTPUT_NAME ncbi-vdb-dll )
    target_link_libraries( ncbi-vdb-shared ncbi-ngs ${LIBXML2} Crypt32 )
    MSVS_StaticRuntime( ncbi-vdb-shared )

    set( VDB_LIB_RD_MD "" )
    foreach( target ${VDB_LIB_RD_TARGETS} )
        list( APPEND VDB_LIB_RD_MD "$<TARGET_OBJECTS:${target}-md>" )
    endforeach()

    add_library( ncbi-vdb-shared-md SHARED "${VDB_LIB_RD_MD}" )
    target_link_libraries( ncbi-vdb-shared-md ncbi-ngs-md ${LIBXML2} Crypt32 )
    MSVS_DLLRuntime( ncbi-vdb-shared-md )

#TODO: rename dlls

else()
    add_library( ncbi-vdb-shared SHARED "${VDB_LIB_RD_SHARED}" )
    set_target_properties( ncbi-vdb-shared PROPERTIES OUTPUT_NAME ncbi-vdb )
    target_link_libraries( ncbi-vdb-shared ncbi-ngs ${LIBXML2} m dl pthread )
    MakeLinksShared( ncbi-vdb-shared ncbi-vdb )
endif()


if( WIN32 )
    add_library( ncbi-wvdb STATIC "${VDB_LIB_RDWR}" )
    #target_link_libraries( ncbi-wvdb Crypt32 )
    MSVS_StaticRuntime( ncbi-wvdb )
    ExportStatic( ncbi-wvdb )

    set( VDB_LIB_RDWR_MD "" )
    foreach( target ${VDB_LIB_RDWR_TARGETS} )
        list( APPEND VDB_LIB_RDWR_MD "$<TARGET_OBJECTS:${target}-md>" )
    endforeach()

    add_library( ncbi-wvdb-md STATIC "${VDB_LIB_RDWR_MD}" )
    #target_link_libraries( ncbi-wvdb-md Crypt32 )
    MSVS_DLLRuntime( ncbi-wvdb-md )
    ExportStatic( ncbi-wvdb-md )
else()
    add_library( ncbi-wvdb STATIC "${VDB_LIB_RDWR}" )
    ExportStatic( ncbi-wvdb )
endif()


# dll
#ExportShared( ncbi-wvdb )
#target_link_libraries( ncbi-wvdb-shared ncbi-ngs ${LIBXML2} m dl pthread )

if(WIN32)
    add_library( ncbi-wvdb-shared SHARED "${VDB_LIB_RDWR}" )
#    set_target_properties( ncbi-wvdb-shared PROPERTIES OUTPUT_NAME ncbi-wvdb-dll )
    target_link_libraries( ncbi-wvdb-shared ncbi-ngs ${LIBXML2} Crypt32 )
    MSVS_StaticRuntime( ncbi-wvdb-shared )

    set( VDB_LIB_RDWR_MD "" )
    foreach( target ${VDB_LIB_RDWR_TARGETS} )
        list( APPEND VDB_LIB_RDWR_MD "$<TARGET_OBJECTS:${target}-md>" )
    endforeach()

    add_library( ncbi-wvdb-shared-md SHARED "${VDB_LIB_RDWR_MD}" )
    target_link_libraries( ncbi-wvdb-shared-md ncbi-ngs-md ${LIBXML2} Crypt32 )
    MSVS_DLLRuntime( ncbi-wvdb-shared-md )

#TODO: rename dlls

else()
    add_library( ncbi-wvdb-shared SHARED "${VDB_LIB_RDWR}" )
    set_target_properties( ncbi-wvdb-shared PROPERTIES OUTPUT_NAME ncbi-wvdb )
    target_link_libraries( ncbi-wvdb-shared ncbi-ngs ${LIBXML2} m dl pthread )
    MakeLinksShared( ncbi-wvdb-shared ncbi-wvdb )
endif()
