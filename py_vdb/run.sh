#!/bin/bash

exec()
{
    echo $1
    eval $1
    echo .
}

ACC="SRR13340286"

SRC="../../plain_tables/$ACC"
TMP="tmp"
OUT="$ACC.copy.sra"

rm -rf $TMP $OUT $OUT.md5 $OUT.*

exec "vdb-dump $SRC --info"

exec "./L11-table_copy.py $SRC -O $TMP"

exec "kar --md5 -f -c $OUT -d $TMP"

rm -rf $TMP $OUT.md5

exec "vdb-diff $SRC $OUT -C READ,QUALITY,READ_LEN,READ_START -p"

exec "vdb-dump $OUT --vdb-blobs"

echo "----------------------------------------------------"
echo "the original file:"
ls -lh $SRC/$ACC.sra
echo .
echo "the copy:"
ls -lh $OUT
echo .
echo "further compression with zstd,bzip2,gzip,brotli,lz4:"
zstd $OUT -f9 --no-progress -q
ls -lh $OUT.zst
bzip2 $OUT -zfk9
ls -lh $OUT.bz2
gzip $OUT -kf9
ls -lh $OUT.gz
brotli -kf9 $OUT
ls -lh $OUT.br
lz4 -zfk9q $OUT
ls -lh $OUT.lz4
