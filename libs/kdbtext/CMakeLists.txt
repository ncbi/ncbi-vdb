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

if( FLEX_FOUND AND BISON_FOUND )
    FLEX_TARGET( KDBTextFlex kdbtext-lex.l ${CMAKE_CURRENT_SOURCE_DIR}/zz_kdbtext-lex.c
        COMPILE_FLAGS "--debug --noline" )
    set( KDBTextScanner ${FLEX_KDBTextFlex_OUTPUTS} )
    BISON_TARGET( KDBTextGrammar kdbtext-grammar.y ${CMAKE_CURRENT_SOURCE_DIR}/zz_kdbtext-grammar.cpp
        COMPILE_FLAGS "-Wno-other --no-lines -r state -v"
        DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/zz_kdbtext-grammar.hpp
        )
   set( KDBTextParser ${BISON_KDBTextGrammar_OUTPUT_SOURCE} )
   ADD_FLEX_BISON_DEPENDENCY(KDBTextFlex KDBTextGrammar)
   SET_DIRECTORY_PROPERTIES(PROPERTIES CLEAN_NO_CUSTOM 1)
else()
    set( KDBTextScanner zz_kdbtext-lex.c )
    set( KDBTextParser zz_kdbtext-grammar.cpp )
endif()

set( KDBTEXT_SRC
    manager.cpp
    ${KDBTextScanner}
#    ${KDBTextParser}
)

GenerateStaticLibs( kdbtext "${KDBTEXT_SRC}" )

add_compile_definitions (__mod__="libs/kdbtext")