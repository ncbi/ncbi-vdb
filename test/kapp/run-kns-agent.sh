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

TOOL_PATH="$1"
RUN_PATH="$2" # a link will be created at that path and the tool will be executed via link
EXPECTED_TOOL_NAME="$3"

RUN_DIR=$(dirname "$RUN_PATH")
RUN_FILE=$(basename "$RUN_PATH")

mkdir -p "$RUN_DIR"
if [ "$?" != "0" ] ; then
    echo "cannot create $RUN_DIR"
    exit 1
fi

CMD="ln -Fs $TOOL_PATH $RUN_PATH"
eval $CMD

USER_AGENT=$("$RUN_PATH")
if [ "$?" != "0" ] ; then
    echo "tool execution failed"
    exit 2
fi
ACTUAL_TOOL_NAME=$(eval "echo $USER_AGENT | awk '{print \$3}'")

if [ "$EXPECTED_TOOL_NAME" != "$ACTUAL_TOOL_NAME" ] ; then
    echo "Agent tool name '$ACTUAL_TOOL_NAME' does not match expected '$EXPECTED_TOOL_NAME'"
    exit 3
fi
