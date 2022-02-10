# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

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
    FLEX_TARGET( SamExtractFlex samextract-lex.l ${CMAKE_CURRENT_SOURCE_DIR}/zz_samextract-lex.c
        COMPILE_FLAGS "--debug --noline" )
    set( SamExtractScanner ${FLEX_SamExtractFlex_OUTPUTS} )
    BISON_TARGET( SamExtractGrammar samextract-grammar.y ${CMAKE_CURRENT_SOURCE_DIR}/zz_samextract-grammar.c
        COMPILE_FLAGS "-Wno-other --no-lines -r state -v --defines=${CMAKE_CURRENT_SOURCE_DIR}/zz_samextract-grammar.h" )
    set( SamExtractParser ${BISON_SamExtractGrammar_OUTPUT_SOURCE} )
    ADD_FLEX_BISON_DEPENDENCY(SamExtractFlex SamExtractGrammar)
    SET_DIRECTORY_PROPERTIES(PROPERTIES CLEAN_NO_CUSTOM 1)
else()
    set( SamExtractScanner zz_samextract-lex.c )
    set( SamExtractParser zz_samextract-grammar.c )
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
ExportStatic( sam-extract true )

GenerateStaticLibs( align-reader "${ALIGN_READER_SRC}" )

add_compile_definitions( __mod__="libs/align" )
