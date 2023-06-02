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
    FLEX_TARGET( SchemaFlex schema-lex.l ${CMAKE_CURRENT_SOURCE_DIR}/zz_schema-lex.c
        COMPILE_FLAGS "--debug --noline" )
    set( SchemaScanner ${FLEX_SchemaFlex_OUTPUTS} )
    BISON_TARGET( SchemaGrammar schema-grammar.y ${CMAKE_CURRENT_SOURCE_DIR}/zz_schema-grammar.cpp
        COMPILE_FLAGS "-Wno-other --no-lines -r state -v"
        DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/zz_schema-grammar.hpp
        )
    set( SchemaParser ${BISON_SchemaGrammar_OUTPUT_SOURCE} )
    ADD_FLEX_BISON_DEPENDENCY(SchemaFlex SchemaGrammar)
    SET_DIRECTORY_PROPERTIES(PROPERTIES CLEAN_NO_CUSTOM 1)
else()
    set( SchemaScanner zz_schema-lex.c )
    set( SchemaParser zz_schema-grammar.cpp )
endif()

# AST parser
if( BISON_FOUND )
    BISON_TARGET( SchemaAstGrammar schema-ast.y ${CMAKE_CURRENT_SOURCE_DIR}/zz_schema-ast.cpp
        COMPILE_FLAGS "-Wno-other --no-lines -r state -v --defines=zz_schema-ast.hpp"
        DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/zz_schema-ast.hpp
    )
    set( SchemaAstParser ${BISON_SchemaAstGrammar_OUTPUT_SOURCE} zz_schema-ast.hpp )
    SET_DIRECTORY_PROPERTIES(PROPERTIES CLEAN_NO_CUSTOM 1)
else()
    set( SchemaAstParser zz_schema-ast.cpp )
endif()

set( SRC
    AST.cpp
    AST_Expr.cpp
    ASTBuilder-db.cpp
    ASTBuilder-func.cpp
    ASTBuilder-phys.cpp
    ASTBuilder-tbl.cpp
    ASTBuilder-type.cpp
    ASTBuilder-view.cpp
    ASTBuilder.cpp
    ErrorReport.cpp
    ParseTree.cpp
    ${SchemaAstParser}
    ${SchemaParser}
    ${SchemaScanner}
    SchemaParser.cpp
    SchemaScanner.cpp
    Token.cpp
)

GenerateStaticLibs( schema "${SRC}" )

if ( WIN32 )
    target_compile_options( schema PRIVATE /EHsc )
else()
    target_compile_options( schema PRIVATE -fno-exceptions -Wno-unused-variable)
endif()

add_compile_definitions (__mod__="libs/schema")
