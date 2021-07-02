add_subdirectory(kfc)

set( COMMON_LIBS_READ
    kapp
    ktst
    ncbi-vdb
    dl
    pthread
)

set( COMMON_LIBS_WRITE
    kapp
    ktst
    ncbi-wvdb
    dl
    pthread
)

#link_libraries( ktst kapp pthread )

add_subdirectory(ktst)
add_subdirectory(cc)
add_subdirectory(ext)
add_subdirectory(judy_test)
add_subdirectory(kfs)
add_subdirectory(klib)
add_subdirectory(kfg)
#add_subdirectory(kapp) # TODO: undefined reference to `KFileMakeBzip2ForRead' in kfs
add_subdirectory(cloud)
add_subdirectory(kdb)
add_subdirectory(kproc)
add_subdirectory(vdb)
# add_subdirectory(search)
# add_subdirectory(sraxf)
# add_subdirectory(vxf)
# add_subdirectory(loader)
# add_subdirectory(krypto)
# add_subdirectory(cipher)
# add_subdirectory(ncbi-vdb)
# add_subdirectory(ngs-python)
# add_subdirectory(kns)
# add_subdirectory(schema)
# add_subdirectory(vfs)
#add_subdirectory(ngs-java)
# add_subdirectory(install)