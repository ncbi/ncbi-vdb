# TODO: INT_LIBS/EXT_LIBS
add_subdirectory(align) # TODO: flex & bison and include dirs order (strcasecmp)
add_subdirectory(ascp)
add_subdirectory(axf) # sra tools dependency
add_subdirectory(blast)
add_subdirectory(cloud)
add_subdirectory(diagnose)
add_subdirectory(ext)
add_subdirectory(hdf5)
add_subdirectory(kapp)
#add_subdirectory(kdb) TODO: split Read and Write versions
add_subdirectory(kfc)
add_subdirectory(kfg)
#add_subdirectory(kfs) # TODO: Makefile has three target libs: libkfs libkfs-nommap libkff
#add_subdirectory(kns) # TODO: build only *.c listed in the Makefile (check for every subdirectory)
add_subdirectory(kproc)
#add_subdirectory(krypto) # same as XFS (3rd priority)
add_subdirectory(kxml) # TODO: includes
add_subdirectory(klib)
add_subdirectory(loader)
#add_subdirectory(ngs) # 2nd priority - dependency on ngs repository
add_subdirectory(schema)
add_subdirectory(search)
add_subdirectory(sra)
add_subdirectory(sraxf) # same as axf
add_subdirectory(svdb)
add_subdirectory(tui)
add_subdirectory(vdb)
add_subdirectory(vdb-sqlite) # includes
#add_subdirectory(vfs) # TODO: build only *.c listed in the Makefile (check for every subdirectory)
#add_subdirectory(vxf) # Sync with Makefile
add_subdirectory(wgsxf)
add_subdirectory(xfs) # 3rd priority
