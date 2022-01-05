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
    list( APPEND VDB_LIB_CMN_TARGETS "kxml" )
    set( LIBXML2 "xml2")
else()
    set( LIBXML2 "")
endif()

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

add_compile_definitions (__mod__="libs/ncbi-vdb" PKGNAME="ncbi-vdb")

if( WIN32 )
    include( CMakeLists.win.txt )
elseif( SINGLE_CONFIG )
    include( CMakeLists.posix.txt )
else() # XCode
    include( CMakeLists.xcode.txt )
endif()

################################
# additional files to install

set( SHARED_SRC_DIR ${CMAKE_INSTALL_PREFIX}/vdb_shared_sources )

message("Installing ncbi-vdb interfaces (headers) into ${CMAKE_INSTALL_PREFIX}")
message("Installing ncbi-vdb shared sources into ${SHARED_SRC_DIR}")

install( DIRECTORY ${CMAKE_SOURCE_DIR}/interfaces DESTINATION ${CMAKE_INSTALL_PREFIX} )
install( FILES
	${CMAKE_SOURCE_DIR}/libs/vdb/blob-priv.h
	${CMAKE_SOURCE_DIR}/libs/vdb/page-map.h
	DESTINATION ${SHARED_SRC_DIR}/libs/vdb )
install( FILES
	${CMAKE_SOURCE_DIR}/libs/kfg/default.kfg
	DESTINATION ${SHARED_SRC_DIR}/libs/kfg )
install( FILES
	${CMAKE_SOURCE_DIR}/libs/align/samextract-pool.h
	${CMAKE_SOURCE_DIR}/libs/align/samextract.h
	DESTINATION ${SHARED_SRC_DIR}/libs/align )
install( FILES
	${CMAKE_SOURCE_DIR}/libs/klib/release-vers.h
	DESTINATION ${SHARED_SRC_DIR}/libs/klib )
install( FILES
	${CMAKE_SOURCE_DIR}/libs/vfs/json-response.h
	${CMAKE_SOURCE_DIR}/libs/vfs/services-priv.h
	DESTINATION ${SHARED_SRC_DIR}/libs/vfs )


