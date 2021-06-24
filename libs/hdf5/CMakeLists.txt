find_package(HDF5 COMPONENTS)

if( HDF5_FOUND )
    add_library(hdf5 STATIC
        hdf5arrayfile.c
        hdf5dir.c
        hdf5file.c
    )

    add_definitions (-D__mod__="libs/hdf5")
endif()