if( LibXml2_FOUND  )
    set( KXML "$<TARGET_OBJECTS:kxml>")
    set( LIBXML2 "xml2")
else()
    set( KXML "")
    set( LIBXML2 "")
endif()

set( VDB_LIB_CMN
    $<TARGET_OBJECTS:align-access>
    $<TARGET_OBJECTS:ncbi-bam>
    $<TARGET_OBJECTS:vfs>
    $<TARGET_OBJECTS:cloud>
    ${KXML}
    $<TARGET_OBJECTS:mbedx509>
    $<TARGET_OBJECTS:mbedtls>
    $<TARGET_OBJECTS:mbedcrypto>
    $<TARGET_OBJECTS:kns>
    $<TARGET_OBJECTS:kfg>
    $<TARGET_OBJECTS:krypto>
    $<TARGET_OBJECTS:ksrch>
    $<TARGET_OBJECTS:kfs>
    $<TARGET_OBJECTS:klib>
    $<TARGET_OBJECTS:judy>
    $<TARGET_OBJECTS:judy1>
    $<TARGET_OBJECTS:judyL>
    $<TARGET_OBJECTS:kproc>
    $<TARGET_OBJECTS:kq>
    $<TARGET_OBJECTS:kfc>
    $<TARGET_OBJECTS:bz2>
    $<TARGET_OBJECTS:z>
    $<TARGET_OBJECTS:zstd>
)

set( VDB_LIB_RD
    ${VDB_LIB_CMN}
    $<TARGET_OBJECTS:align-reader>
    $<TARGET_OBJECTS:ascp>
    $<TARGET_OBJECTS:srareader>
    $<TARGET_OBJECTS:sradb>
    $<TARGET_OBJECTS:vdb-blast>
    $<TARGET_OBJECTS:vdb>
    $<TARGET_OBJECTS:kdb>
    $<TARGET_OBJECTS:axf>
    $<TARGET_OBJECTS:sraxf>
    $<TARGET_OBJECTS:wgsxf>
    $<TARGET_OBJECTS:vxf>
)

set( VDB_LIB_RDWR
    ${VDB_LIB_CMN}
    $<TARGET_OBJECTS:align-writer>
    $<TARGET_OBJECTS:wsradb>
    $<TARGET_OBJECTS:wvdb>
    $<TARGET_OBJECTS:wkdb>
    $<TARGET_OBJECTS:waxf>
    $<TARGET_OBJECTS:wsraxf>
    $<TARGET_OBJECTS:wwgsxf>
    $<TARGET_OBJECTS:wvxf>
)

add_library( ncbi-ngs STATIC
    $<TARGET_OBJECTS:ngs>
    $<TARGET_OBJECTS:ngs-jni>
    $<TARGET_OBJECTS:ngs-py>
)

add_library( ncbi-ngs-c++ STATIC
    $<TARGET_OBJECTS:ngs-c++>
)

# TODO: add_library ncbi-ngs-c++


add_library( ncbi-vdb STATIC ${VDB_LIB_RD} )
add_library( ncbi-wvdb STATIC ${VDB_LIB_RDWR} )

add_library( ncbi-vdb-shared SHARED
            ${VDB_LIB_RD}
            $<TARGET_OBJECTS:ngs>
            $<TARGET_OBJECTS:ngs-jni>
            $<TARGET_OBJECTS:ngs-py>
)

set_target_properties( ncbi-vdb-shared PROPERTIES OUTPUT_NAME ncbi-vdb )
target_link_libraries( ncbi-vdb-shared ncbi-ngs ${LIBXML2} )

add_library( ncbi-wvdb-shared SHARED ${VDB_LIB_RDWR} )
set_target_properties( ncbi-wvdb-shared PROPERTIES OUTPUT_NAME ncbi-wvdb )

