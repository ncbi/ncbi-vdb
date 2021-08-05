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
