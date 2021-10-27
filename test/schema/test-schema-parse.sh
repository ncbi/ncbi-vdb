#!/bin/bash

test_schema_parse=$1
TOP=$2

SCHEMATA=$(find $TOP -name "*.vschema")

echo Starting Schema Parse test

output=$($test_schema_parse $SCHEMATA)
res=$?
if [ "$res" != "0" ];
	then echo "Schema Parse test FAILED, res=$res output=$output" && exit 1;
fi

echo Schema Parse test is finished
