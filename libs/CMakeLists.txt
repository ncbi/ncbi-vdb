set( CMAKE_POSITION_INDEPENDENT_CODE True )
add_compile_definitions( _LIBRARY )

# TODO: INT_LIBS/EXT_LIBS
add_subdirectory(ktst)
add_subdirectory(align) 
add_subdirectory(ascp)
add_subdirectory(axf) # sra tools dependency
add_subdirectory(blast)
add_subdirectory(cloud)
add_subdirectory(diagnose)
add_subdirectory(ext)
add_subdirectory(hdf5)
add_subdirectory(kapp)
add_subdirectory(kdb)
add_subdirectory(kfc)
add_subdirectory(kfg) 
add_subdirectory(kfs)
add_subdirectory(klib) # TODO: Judy is not ready
add_subdirectory(kns) 
add_subdirectory(kproc)
add_subdirectory(krypto) # TODO: to be replaced with calls to mbedtls
add_subdirectory(kxml) # TODO: includes
add_subdirectory(loader)
#add_subdirectory(ngs) # 2nd priority - dependency on ngs repository
add_subdirectory(schema)
add_subdirectory(search)
add_subdirectory(sra)
add_subdirectory(sraxf) # same as axf
add_subdirectory(svdb)
add_subdirectory(tui)
add_subdirectory(vdb)
add_subdirectory(vdb-sqlite)
add_subdirectory(vfs)
add_subdirectory(vxf)
add_subdirectory(wgsxf) # wgsxf and wwgsxf are the same
#add_subdirectory(xfs) # 3rd priority

add_subdirectory(ncbi-vdb)
