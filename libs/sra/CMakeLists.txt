set( SRADBCMN_SRC
    sramgr-cmn
    sratbl-cmn
    sracol
    types
    sracache
)

set( SRADB_SRC
    ${SRADBCMN_SRC}
    sramgr
    sratbl
)

set( WSRADB_SRC
    ${SRADBCMN_SRC}
    wsramgr
    wsratbl
)

set( SRAREADER_SRC
    reader-cmn
    reader-fastq
    reader-sff
    reader-illumina
    reader-abi
)

GenerateStaticLibs( sradb "${SRADB_SRC}" )
GenerateStaticLibs( wsradb "${WSRADB_SRC}" )
GenerateStaticLibs( srareader "${SRAREADER_SRC}" )

add_compile_definitions (__mod__="libs/sra")