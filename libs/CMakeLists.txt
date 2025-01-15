# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

set( CMAKE_POSITION_INDEPENDENT_CODE True )
add_compile_definitions( _LIBRARY )

add_subdirectory(ktst)
add_subdirectory(align)
add_subdirectory(axf) # sra tools dependency
add_subdirectory(blast)
add_subdirectory(cloud)
add_subdirectory(ext)
add_subdirectory(kapp)
add_subdirectory(kdb)
add_subdirectory(kdbtext)
add_subdirectory(kfc)
add_subdirectory(kfg)
add_subdirectory(kfs)
add_subdirectory(klib)
add_subdirectory(kns)
add_subdirectory(kproc)
add_subdirectory(krypto) # TODO: to be replaced with calls to mbedtls
add_subdirectory(schema)
add_subdirectory(sra)
add_subdirectory(sraxf) # same as axf
add_subdirectory(vdb)
add_subdirectory(vfs)
add_subdirectory(vxf)
add_subdirectory(wgsxf) # wgsxf and wwgsxf are the same
add_subdirectory(ncbi-vdb)

if ( SINGLE_CONFIG OR _NCBIVDB_CFG_PACKAGING )
    #
    # Install header files
    set( HEADERS align  cloud  insdc  kdb  kfg  klib  kproc   ktst  os sra   vfs
    cc     csra2  ext  kapp   kfc  kfs  kns   krypto  ncbi  vdb  )

    foreach( h IN ITEMS ${HEADERS} )
        install( DIRECTORY ${CMAKE_SOURCE_DIR}/interfaces/${h} DESTINATION ${INST_INCDIR} )
    endforeach()
endif()
if(_NCBIVDB_CFG_PACKAGING)
    install( FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION licenses)
endif()

#add_subdirectory(kxml) VDB-4797 moved to sra-tools
#add_subdirectory(loader) VDB-4855 moved to sra-tools/libs
#add_subdirectory(hdf5) VDB-4795
#add_subdirectory(tui) VDB-4803
#add_subdirectory(diagnose) VDB-4794 moved to sra-tools/test-sra
#add_subdirectory(ascp) VDB-4794 moved to sra-tools/libs/ascp
