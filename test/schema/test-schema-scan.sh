#!/bin/bash

test_schema_scan=$1
TOP=$2

SCHEMATA=$(find $TOP/interfaces -type f -name "*.vschema" -not -name ".*")

echo Starting Schema Scan test

output=$($test_schema_scan $SCHEMATA)
res=$?
if [ "$res" != "0" ];
	then echo "Schema scanner test FAILED, res=$res output=$output" && exit 1;
fi

echo Schema Scan test is finished
