#!/bin/bash

cd $(dirname $0)/.. || exit 1
TOP=$(pwd)

cd $TOP/interfaces/ext || exit 2
for f in $(find mbedtls psa -name "*.h")
do
    cp -a $f $f.orig || exit 3
done

cd $TOP/libs/ext || exit 4
for f in $(find mbedtls -name "*.[ch]")
do
    cp -a $f $f.orig || exit 5
done
