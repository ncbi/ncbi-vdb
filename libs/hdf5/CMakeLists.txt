find_package(HDF5 COMPONENTS)

if( HDF5_FOUND )
    add_library(kdf5 STATIC
        hdf5arrayfile.c
        hdf5dir.c
        hdf5file.c
    )

    add_compile_definitions (__mod__="libs/hdf5")
    include_directories( ${HDF5_INCLUDE_DIRS} )

    ExportStatic( kdf5 )
    ExportShared( kdf5 )
endif()