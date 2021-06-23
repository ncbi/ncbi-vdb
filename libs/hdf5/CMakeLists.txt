add_library(hdf5 STATIC
    hdf5arrayfile.c
    hdf5dir.c
    hdf5file.c
)

add_definitions (-D__mod__="libs/hdf5")