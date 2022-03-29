#!/bin/bash

DIRTOTEST=$1

ACCESSION=SRR072810
URL=https://sra-pub-run-odp.s3.amazonaws.com/sra/SRR072810/SRR072810
EXPECTED_MD5="a2ac49a4c516e77376cbdae3f4c9a144  ./${ACCESSION}.dat"

if echo ${URL} | grep -vq /sdlr/sdlr.fcgi?jwt=
    then 
        ${DIRTOTEST}/vdb-get --reliable -c ./${ACCESSION}.cachetee ${URL} ${ACCESSION}.dat --progress
        ACTUAL_MD5=$(md5sum ./${ACCESSION}.dat)
        if [ "$EXPECTED_MD5" != "$ACTUAL_MD5" ]
        then
            echo "expected: $EXPECTED_MD5"
            echo "actual:   $ACTUAL_MD5"
            exit 1
        fi
        rm -f ${ACCESSION}*
    else 
        echo kget test when CE is required is skipped ; 
    fi