add_library(sra STATIC
    reader-abi.c
    reader-cmn.c
    reader-fastq.c
    reader-illumina.c
    reader-sff.c
    sra-dflt-schema.c
    sra-schema.c
    sracache.c
    sracol.c
    sramgr-cmn.c
    sramgr.c
    sratbl-cmn.c
    sratbl.c
    types.c
    wsramgr.c
    wsratbl.c
)

add_definitions (-D__mod__="libs/sra")