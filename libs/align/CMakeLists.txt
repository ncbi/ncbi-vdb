set ( BAM_SRC bam )
set ( ALIGN_ACCESS_SRC align-access )
set ( ALIGN_READER_SRC
    align_mgr
    al_iterator
    unsupported_pileup_estimator
    ref_iterator
    pl_iterator
    dna-reverse-cmpl
    reference-cmn
    reader-cmn
    reader-refseq
    reader-wgs
    reference
    refseq-mgr
    quality-quantizer
)

set ( ALIGN_WRITER_SRC
    dna-reverse-cmpl
    reader-cmn
    reference-cmn
    reader-refseq
    reader-wgs
    refseq-mgr
    writer-cmn
    writer-refseq
    writer-alignment
    writer-sequence
    writer-ref
    writer-reference
)

set ( SAM_EXTRACT_SRC
    samextract-grammar
    samextract-lex
    samextract-lib
    samextract-bam
    samextract-pool
)

add_library ( ncbi-bam STATIC ${BAM_SRC})
add_library ( align-writer STATIC ${ALIGN_WRITER_SRC})
add_library ( align-access STATIC ${ALIGN_ACCESS_SRC})
add_library ( sam-extract STATIC ${SAM_EXTRACT_SRC})
add_library ( align-reader STATIC ${ALIGN_READER_SRC})

# TODO: flex & bison

add_definitions (-D__mod__="libs/align")
#add_definitions (-D_ARCH_BITS=${BITS})

