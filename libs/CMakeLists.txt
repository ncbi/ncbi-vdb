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
add_subdirectory(klib)
add_subdirectory(kns)
add_subdirectory(kproc)
add_subdirectory(krypto) # TODO: to be replaced with calls to mbedtls
add_subdirectory(kxml)
add_subdirectory(loader)
add_subdirectory(schema)
add_subdirectory(search)
add_subdirectory(sra)
add_subdirectory(sraxf) # same as axf
add_subdirectory(svdb)
add_subdirectory(tui)
add_subdirectory(vdb)
add_subdirectory(vfs)
add_subdirectory(vxf)
add_subdirectory(wgsxf) # wgsxf and wwgsxf are the same
add_subdirectory(ncbi-vdb)

#add_subdirectory(xfs) # dead
