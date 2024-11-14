# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

# all static libraries in this directory are published

set( VDB_LIB_CMN_TARGETS
    align-access
    ncbi-bam
    vfs
    cloud
    kns
    kfg
    krypto
    kfs
    klib
    judy
    kproc
    kq
    kfc
    schema
    kdb
)
if ( HAVE_MBEDTLS_F )
    message("Using mbedtls installed in the system")
else()
    message("No mbedtls libs found installed in the system, using local copy...")
    list( APPEND VDB_LIB_CMN_TARGETS mbedx509 mbedtls mbedcrypto )
endif()

if(NOT _NCBIVDB_CFG_PACKAGING)
    list( APPEND VDB_LIB_CMN_TARGETS bz2 z zstd )
endif()

set( VDB_LIB_RD_TARGETS
    ${VDB_LIB_CMN_TARGETS}
    align-reader
    srareader
    sradb
    vdb-blast
    vdb
    axf
    sraxf
    wgsxf
    vxf
)

set( VDB_LIB_RDWR_TARGETS
    ${VDB_LIB_CMN_TARGETS}
    align-writer
    wsradb
    wvdb
    waxf
    wsraxf
    wwgsxf
    wvxf
)

if( WIN32 )
    include( CMakeLists.win.txt )
elseif( SINGLE_CONFIG )
    include( CMakeLists.posix.txt )
    if(NOT _NCBIVDB_CFG_PACKAGING)
	add_custom_command( TARGET ncbi-vdb POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/interfaces/kfg/ncbi/default.kfg ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ncbi/default.kfg
	)
	add_custom_command( TARGET ncbi-vdb POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/interfaces/kfg/ncbi/default.kfg ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/ncbi/default.kfg
	)
	add_custom_command( TARGET ncbi-vdb POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/interfaces/kfg/ncbi/certs.kfg ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ncbi/certs.kfg
	)
	add_custom_command( TARGET ncbi-vdb POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/interfaces/kfg/ncbi/certs.kfg ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/ncbi/certs.kfg
	)
    endif()
else() # XCode
    list(REMOVE_ITEM VDB_LIB_RD_TARGETS bz2 z)
    list(REMOVE_ITEM VDB_LIB_RDWR_TARGETS bz2 z)
    include( CMakeLists.xcode.txt )
endif()
