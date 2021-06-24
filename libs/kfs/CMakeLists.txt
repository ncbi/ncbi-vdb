set ( UNIX_SRC unix/sysdir.c unix/sysdll.c unix/sysfile.c unix/syslockfile.c unix/sysmmap.c )
set ( LINUX_SRC ${UNIX_SRC} )
set ( MAC_SRC ${UNIX_SRC} )
set ( WIN_SRC win/directory-path.c win/sysdir.c win/sysdll.c win/sysfile.c win/syslockfile.c win/sysmmap.c
#win/lnk_tools.c
)

set ( UNIX_SRC_V2 unix/sysfile-v2.c )
set ( LINUX_SRC_V2 ${UNIX_SRC_V2} )
set ( MAC_SRC_V2 ${UNIX_SRC_V2} )
set ( WIN_SRC_V2 win/sysfile-v2.c )


if ( "win" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${WIN_SRC} )
    include_directories( win )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${LINUX_SRC} )
    include_directories( unix )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC ${MAC_SRC} )
    include_directories( unix )
endif()


if ( "win" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC_V2 ${WIN_SRC_V2} )
    include_directories( win )
elseif( "linux" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC_V2 ${LINUX_SRC_V2} )
    include_directories( unix )
elseif( "mac" STREQUAL ${OS} )
    set ( OS_SPECIFIC_SRC_V2 ${MAC_SRC_V2} )
    include_directories( unix )
endif()

set( KFS_CMN
    directory
    arrayfile
    file
    chunk-reader
    gzip
    bzip
    md5
    crc32
    arc
    toc
    tocentry
    tocdir
    tocfile
    sra
    tar
    teefile
    buffile
    buffile-read
    buffile-write
    subfile
    nullfile
    countfile
    dir_test
    pagefile
    pmem
    readheadfile
    ramfile
    extendable-ramfile
    quickmount
    report
    lockfile
    remove-file-task
    cacheteefile
    cacheteefile2
    cacheteefile3
    rrcachedfile
    recorder
    logfile
    callback_file
    poolpages
    lru_cache
    from_to_namelist
    limitfile
    appendfile
    mmap
    ${OS_SPECIFIC_SRC}
)

set( KFS_SRC
    ${KFS_CMN}
)

set( KFS2_SRC
    ${KFS_SRC_V2}
    file-v2
    directory-v2
)

set( KFS_NOMMAP_SRC
    ${KFS_CMN}
)
add_library( kfs-nommap STATIC ${KFS_NOMMAP_SRC} )
target_compile_definitions(kfs-nommap PRIVATE FORCE_MALLOC_MMAP )

set( KFF_SRC
    fileformat
    ffext
    ffkey
    ffmagic
)

add_library( kfs STATIC ${KFS_SRC} )
#add_library( kfs.2 STATIC ${KFS2_SRC} )
add_library( kff STATIC ${KFF_SRC} )

add_definitions (-D__mod__="libs/kfs")
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )