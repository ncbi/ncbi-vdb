#!/bin/bash
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

#echo $*

# install-root.sh
#   if running as root, install:
#       symlink /usr/include/ncbi-vdb -> $2 (INST_INCDIR from CMake)
#       /etc/profile.d/ncbi-vdb.sh and
#       /etc/profile.d/ncbi-vdb.csh,
#          both containing update to LD_LIBRARY_PATH and setting NCBI_VDB_LIBDIR to $3 (INST_LIBDIR from CMake)

VERSION=$1
INCDIR=$2
LIBDIR=$3
INCLUDE_SYMLINK=/usr/include/ncbi-vdb
PROFILE_FILE=/etc/profile.d/ncbi-vdb

if [ "$EUID" -eq 0 ]; then

    echo "Updating ${INCLUDE_SYMLINK}"
    rm -f ${INCLUDE_SYMLINK}
    ln -s ${INCDIR} ${INCLUDE_SYMLINK}

    echo "Updating ${PROFILE_FILE}.sh"
    printf \
"#version ${VERSION}\n"\
"if ! echo \$LD_LIBRARY_PATH | /bin/grep -q ${LIBDIR}\n"\
"then export LD_LIBRARY_PATH=${LIBDIR}:\$LD_LIBRARY_PATH\n"\
"fi\n"\
"export NCBI_VDB_LIBDIR=${LIBDIR}\n" \
        >${PROFILE_FILE}.sh && chmod 644 ${PROFILE_FILE}.sh

    echo "Updating ${PROFILE_FILE}.csh"
    printf \
"#version ${VERSION}\n"\
"echo \$LD_LIBRARY_PATH | /bin/grep -q ${LIBDIR}\n"\
"if ( \$status ) setenv LD_LIBRARY_PATH ${LIBDIR}:\$LD_LIBRARY_PATH\n"\
"setenv NCBI_VDB_LIBDIR ${LIBDIR}\n" \
        >${PROFILE_FILE}.csh && chmod 644 ${PROFILE_FILE}.csh

fi
