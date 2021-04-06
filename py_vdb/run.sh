#!/bin/bash

exec()
{
    echo $1
    eval $1
    echo .
}


ACC=$1

SRC="./$ACC/$ACC.sra"
TMP="tmp"
OUT1="$ACC.copy_1.sra"
OUT2="$ACC.copy_2.sra"
SCHEMA1="tbl_copy_1.vschema"
SCHEMA2="tbl_copy_2.vschema"

rm -rf $TMP $OUT1 $OUT1.md5 $OUT1.* $OUT2 $OUT2.md5 $OUT2.*

exec "prefetch -p $ACC"

exec "./L11-table_copy.py $SRC -O $TMP -S $SCHEMA1"
exec "kar --md5 -f -c $OUT1 -d $TMP"
rm -rf $TMP $OUT1.md5

exec "./L11-table_copy.py $SRC -O $TMP -S $SCHEMA2"
exec "kar --md5 -f -c $OUT2 -d $TMP"
rm -rf $TMP $OUT2.md5

./record_sizes.py $ACC -O $SRC -1 $OUT1 -2 $OUT2 -D data.db

rm -rf $OUT1 $OUT2 $ACC

exit 0

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
