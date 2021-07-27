find_package(HDF5 COMPONENTS)

if( HDF5_FOUND )
    set( SRC
        hdf5arrayfile.c
        hdf5dir.c
        hdf5file.c
    )
    GenerateStaticLibs(kdf5 "${SRC}" )

    add_compile_definitions (__mod__="libs/hdf5")
    include_directories( ${HDF5_INCLUDE_DIRS} )

    ExportStatic( kdf5 )
    ExportShared( kdf5 )

    target_link_libraries( kdf5-shared ${HDF5_LIBRARIES} ncbi-vdb)

endif()
