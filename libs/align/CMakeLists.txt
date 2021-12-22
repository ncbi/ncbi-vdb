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

if( FLEX_FOUND AND BISON_FOUND )
    FLEX_TARGET( SamExtractFlex samextract-lex.l ${CMAKE_CURRENT_SOURCE_DIR}/samextract-lex.c
        COMPILE_FLAGS "--debug --noline" )
    set( SamExtractScanner ${FLEX_SamExtractFlex_OUTPUTS} )
    BISON_TARGET( SamExtractGrammar samextract-grammar.y ${CMAKE_CURRENT_SOURCE_DIR}/samextract-grammar.c
        COMPILE_FLAGS "-Wno-other --no-lines -r state" )
    set( SamExtractParser ${BISON_SamExtractGrammar_OUTPUT_SOURCE} )
    ADD_FLEX_BISON_DEPENDENCY(SamExtractFlex SamExtractGrammar)
else()
    set( SamExtractScanner samextract-lex.c )
    set( SamExtractParser samextract-grammar.c )
endif()


set ( SAM_EXTRACT_SRC
    ${SamExtractParser}
    ${SamExtractScanner}
    samextract-lib
    samextract-bam
    samextract-pool
)

GenerateStaticLibs( ncbi-bam "${BAM_SRC}" )
GenerateStaticLibs( align-writer "${ALIGN_WRITER_SRC}" )
GenerateStaticLibs( align-access "${ALIGN_ACCESS_SRC}" )
GenerateStaticLibs( sam-extract "${SAM_EXTRACT_SRC}" )
GenerateStaticLibs( align-reader "${ALIGN_READER_SRC}" )

add_compile_definitions( __mod__="libs/align" )
# add_compile_definitions( _ARCH_BITS=${BITS} )

