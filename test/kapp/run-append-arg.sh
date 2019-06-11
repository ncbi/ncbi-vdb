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
SHKRIPT=`basename $0`

EXECUTABLE=$1


DEBMO=`echo $2 | grep -- "-DNDEBUG" >/dev/null 2>&2 || echo "Y"`
if [ -z "$DEBMO" ]
then
    DEBMO=N
    echo "## TESTING RELEASE VERSION"
else
    echo "## TESTING DEBUG VERSION"
fi


usage ()
{
    cat <<EOF >&2

Syntax: $SHKRIPT executable

Where:

    executable - path to executable to test

EOF
}


if [ -z "$EXECUTABLE" ]
then
    echo ERROR: invalid parameters  >&2
    echo
    usage
    exit 1
fi

if [ ! -x "$EXECUTABLE" ]
then
	echo ERROR: can not stat executable '$EXECUTABLE' >&2
	exit 1
fi

###
##  Test 1 : help contans parameter in help string
###
echo "## 1. Testing argument existence in help string"
CMD="$EXECUTABLE -h"
if [ "$DEBMO" == "Y" ]
then
    RET=`eval $CMD | ( grep -- "--append_output" >/dev/null 2>&1 && echo GOOD )`
else
    RET=`eval $CMD | ( grep -- "--append_output" >/dev/null 2>&1 || echo GOOD )`
fi

if [ "$RET" != "GOOD" ]
then
    echo "## <<== FAILED"
    exit 2
else
    echo "## <<== PASSED"
fi

###
##  Test 2 : help does not contans parameter name
###
echo "## 2. Testing argument absence in help string"
CMD="$EXECUTABLE -h jojoba"
RET=`eval $CMD | ( grep -- "--append_output" >/dev/null 2>&1 || echo GOOD )`
if [ "$RET" != "GOOD" ]
then
    echo "## <<== FAILED"
    exit 2
else
    echo "## <<== PASSED"
fi

###
##  Test 3 : append output true
###
echo "## 3. Testing --append_output argument is true"
CMD="$EXECUTABLE --append_output"
RET=`eval $CMD | ( grep -- "APPEND_MODE: Y" >/dev/null 2>&1 && echo GOOD )`
if [ "$RET" != "GOOD" ]
then
    echo "## <<== FAILED"
    exit 2
else
    echo "## <<== PASSED"
fi

###
##  Test 4 : append output false
###
echo "## 4. Testing --append_output argument is false"
CMD="$EXECUTABLE"
RET=`eval $CMD | ( grep -- "APPEND_MODE: N" >/dev/null 2>&1 && echo GOOD )`
if [ "$RET" != "GOOD" ]
then
    echo "## <<== FAILED"
    exit 2
else
    echo "## <<== PASSED"
fi
