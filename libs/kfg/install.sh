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

# install.sh
#   installs required .kfg files from $1
#   to $2 (non-root) or $3 (root)

SCRIPT_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)
SRC_DIR=$1
KFGSUMS=$SRC_DIR/kfgsums


if [ "$EUID" -eq 0 ]; then
    KONFIG_DIR=$3
    mkdir -p ${KONFIG_DIR}
    cp $KFGSUMS $KONFIG_DIR
else
    KONFIG_DIR=$2
    mkdir -p ${KONFIG_DIR}
fi

$SCRIPT_DIR/install-kfg.sh default.kfg $SRC_DIR $KONFIG_DIR $KFGSUMS
$SCRIPT_DIR/install-kfg.sh certs.kfg   $SRC_DIR $KONFIG_DIR $KFGSUMS
