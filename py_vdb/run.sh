#!/bin/bash

ACC="SRR13340286"

SRC="../../plain_tables/$ACC"
TMP="tmp"
OUT="$ACC.copy.sra"

rm -rf $TMP $OUT $OUT.md5

./L11-read2na.py $SRC -O $TMP
kar --md5 -c $OUT -d $TMP
rm -rf $TMP $OUT.md5

vdb-diff $SRC $OUT -C READ,QUALITY,READ_LEN,READ_START,NAME -p

ls -l $SRC
ls -l $OUT
