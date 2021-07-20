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

if( FLEX_FOUND )
    FLEX_TARGET( SamExtractFlex samextract-lex.l ${CMAKE_CURRENT_SOURCE_DIR}/samextract-lex.c
        COMPILE_FLAGS "--debug --noline" )
    set( SamExtractScanner ${FLEX_SamExtractFlex_OUTPUTS} )
else()
    set( SamExtractScanner samextract-lex.c )
endif()

if( BISON_FOUND )
    BISON_TARGET( SamExtractGrammar samextract-grammar.y ${CMAKE_CURRENT_SOURCE_DIR}/samextract-grammar.c
        COMPILE_FLAGS "-Wno-other --no-lines -r state" )
    set( SamExtractParser ${BISON_SamExtractGrammar_OUTPUT_SOURCE} )
    ADD_FLEX_BISON_DEPENDENCY(SamExtractFlex SamExtractGrammar)
else()
    set( SamExtractParser samextract-grammar.c )
endif()


set ( SAM_EXTRACT_SRC
    ${SamExtractParser}
    ${SamExtractScanner}
    samextract-lib
    samextract-bam
    samextract-pool
)

add_library ( ncbi-bam STATIC ${BAM_SRC})
add_library ( align-writer STATIC ${ALIGN_WRITER_SRC})
add_library ( align-access STATIC ${ALIGN_ACCESS_SRC})
add_library ( sam-extract STATIC ${SAM_EXTRACT_SRC})
add_library ( align-reader STATIC ${ALIGN_READER_SRC})

add_compile_definitions( __mod__="libs/align" )
# add_compile_definitions( _ARCH_BITS=${BITS} )

