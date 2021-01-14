#!/bin/bash

cd $(dirname $0)/.. || exit 1
TOP=$(pwd)

cd $TOP/interfaces/ext || exit 2
for f in $(find mbedtls psa -name "*.h")
do
    sed -f $TOP/build/mbedtls-vdb-namespace.sed $f.orig > $f || exit 3
done

cd $TOP/libs/ext || exit 4
for f in $(find mbedtls -name "*.h")
do
    sed -f $TOP/build/mbedtls-vdb-namespace.sed $f.orig > $f || exit 5
done

for f in $(find mbedtls -name "*.c")
do
    sed -f $TOP/build/mbedtls-lib-name-replace.sed $f.orig > $f || exit 6
done
