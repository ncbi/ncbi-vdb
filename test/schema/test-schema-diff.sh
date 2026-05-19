#!/bin/sh

test_schema_diff=$1
TOP=$2
WORKDIR=$3

echo Starting Schema Diff test

mkdir -p $WORKDIR/data

echo "VDB_CONFIG=local.kfg $test_schema_diff -I$TOP/interfaces -o$WORKDIR/data $TOP/interfaces/vdb/built-in.vschema $TOP/interfaces/vdb/vdb.vschema"

output=$(VDB_CONFIG=local.kfg $test_schema_diff -I$TOP/interfaces -o$WORKDIR/data \
	$TOP/interfaces/vdb/built-in.vschema \
	$TOP/interfaces/vdb/vdb.vschema \
)

res=$?
if [ "$res" != "0" ];
	then echo "Schema Diff test FAILED, res=$res output=$output" && exit 1;
fi

echo Schema Diff test is finished
