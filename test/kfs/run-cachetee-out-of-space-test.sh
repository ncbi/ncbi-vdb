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

RUN_DIR="$1"
BIN_DIR="$2"

echo "Out of space test #1 ... there is no space to create the cache-tee-file from the beginning"
#=====================================================================================================

#try to get rid of fuse-mount ( in case the test was terminated, before this script was able
#to unmount the fuse-mount-point...
fusermount -u $RUN_DIR/mount_point

rm -rf $RUN_DIR/mount_point $RUN_DIR/proxy_point

mkdir $RUN_DIR/mount_point
rc=$?; if [[ $rc -ne 0 ]]; then echo "error creating mount-point"; exit $rc; fi

mkdir $RUN_DIR/proxy_point
rc=$?; if [[ $rc -ne 0 ]]; then echo "error creating proxy-point"; exit $rc; fi

TRUNCATE_LIMIT="10k"
WRITE_LIMIT="0"
$BIN_DIR/fuse-proxy $RUN_DIR/mount_point $RUN_DIR/proxy_point $TRUNCATE_LIMIT $WRITE_LIMIT
rc=$?; if [[ $rc -ne 0 ]]; then echo "error starting fuse-proxy"; exit $rc; fi

$BIN_DIR/test-cachetee-out-of-space
rc_test=$?

fusermount -u $RUN_DIR/mount_point
rc=$?; if [[ $rc -ne 0 ]]; then echo "error stopping fuse-proxy"; exit $rc; fi

rm -rf $RUN_DIR/mount_point $RUN_DIR/proxy_point

if [[ $rc_test -ne 0 ]]; then echo "error in test-cachetee-out-of-space binary"; exit $rc_test; fi


echo "Out of space test #2 ... there is no space while using the cache-tee-file"
#=====================================================================================================
#try to get rid of fuse-mount ( in case the test was terminated, before this script was able
#to unmount the fuse-mount-point...
fusermount -u $RUN_DIR/mount_point

rm -rf $RUN_DIR/mount_point $RUN_DIR/proxy_point

mkdir $RUN_DIR/mount_point
rc=$?; if [[ $rc -ne 0 ]]; then echo "error creating mount-point"; exit $rc; fi

mkdir $RUN_DIR/proxy_point
rc=$?; if [[ $rc -ne 0 ]]; then echo "error creating proxy-point"; exit $rc; fi

TRUNCATE_LIMIT="0"
WRITE_LIMIT="10k"
$BIN_DIR/fuse-proxy $RUN_DIR/mount_point $RUN_DIR/proxy_point $TRUNCATE_LIMIT $WRITE_LIMIT
rc=$?; if [[ $rc -ne 0 ]]; then echo "error starting fuse-proxy"; exit $rc; fi

$BIN_DIR/test-cachetee-out-of-space
rc_test=$?

fusermount -u $RUN_DIR/mount_point
rc=$?; if [[ $rc -ne 0 ]]; then echo "error stopping fuse-proxy"; exit $rc; fi

rm -rf $RUN_DIR/mount_point $RUN_DIR/proxy_point

if [[ $rc_test -ne 0 ]]; then echo "error in test-cachetee-out-of-space binary"; exit $rc_test; fi
