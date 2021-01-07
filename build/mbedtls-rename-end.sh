#!/bin/bash


cd $(dirname $0)/.. || exit 1
TOP=$(pwd)

cd $TOP/interfaces/ext || exit 2
rm $(find mbedtls psa -name "*.orig") || exit 3

cd $TOP/libs/ext || exit 4
rm $(find mbedtls -name "*.orig") || exit 5
