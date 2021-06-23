if ( "win" STREQUAL ${OS} )
    include_directories(win)
    #message ("Adding include directory: " ${CMAKE_CURRENT_SOURCE_DIR}"/libs/kfs/win")
    set (OS_SPECIFIC_SRC win/directory-path.c win/lnk_tools.c win/sysdir.c win/sysdll.c win/sysfile-v2.c win/sysfile.c win/syslockfile.c win/sysmmap.c)
elseif( "unix" STREQUAL ${OS} )
    #include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/kfs/unix)
    #message ("Adding include directory: " ${CMAKE_CURRENT_SOURCE_DIR}"/libs/kfs/win")
    set (OS_SPECIFIC_SRC unix/sysdir.c unix/sysdll.c unix/sysfile-v2.c unix/sysfile.c unix/syslockfile.c unix/sysmmap.c)
elseif( "linux" STREQUAL ${OS} )
    include_directories(unix)
    message ("Adding include directory: " ${CMAKE_CURRENT_SOURCE_DIR}"/libs/kfs/win")
    set (OS_SPECIFIC_SRC unix/sysdir.c unix/sysdll.c unix/sysfile-v2.c unix/sysfile.c unix/syslockfile.c unix/sysmmap.c)
endif()

add_library(kfs STATIC
    appendfile.c
    arc.c
    arrayfile.c
    buffile-read.c
    buffile-write.c
    buffile.c
    bzip.c
    cacheteefile.c
    cacheteefile2.c
    cacheteefile3.c
    callback_file.c
    chunk-reader.c
    countfile.c
    crc32.c
    directory-v2.c
    directory.c
    dir_test.c
    extendable-ramfile.c
    ffext.c
    ffkey.c
    ffmagic.c
    file-v2.c
    file.c
    fileformat.c
    from_to_namelist.c
    gzip.c
    limitfile.c
    lockfile.c
    logfile.c
    lru_cache.c
    manager.c
    md5.c
    mmap.c
    nullfile.c
    pagefile.c
    pmem.c
    poolpages.c
    quickmount.c
    ramfile.c
    readheadfile.c
    recorder.c
    remove-file-task.c
    report.c
    rrcachedfile.c
    sra.c
    subfile.c
    szip.c
    tar.c
    teefile.c
    toc.c
    tocdir.c
    tocentry.c
    tocfile.c
    ${OS_SPECIFIC_SRC}
)

add_definitions (-D__mod__="libs/kfs")