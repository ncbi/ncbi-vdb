if( FLEX_FOUND AND BISON_FOUND )
    FLEX_TARGET( SchemaFlex schema-lex.l ${CMAKE_CURRENT_SOURCE_DIR}/schema-lex.c
        COMPILE_FLAGS "--debug --noline" )
    set( SchemaScanner ${FLEX_SchemaFlex_OUTPUTS} )
    BISON_TARGET( SchemaGrammar schema-grammar.y ${CMAKE_CURRENT_SOURCE_DIR}/schema-grammar.cpp
        COMPILE_FLAGS "-Wno-other --no-lines -r state -v" )
    set( SchemaParser ${BISON_SchemaGrammar_OUTPUT_SOURCE} )
    ADD_FLEX_BISON_DEPENDENCY(SchemaFlex SchemaGrammar)
else()
    set( SchemaScanner schema-lex.c )
    set( SchemaParser schema-grammar.cpp )
endif()

# AST parser
if( BISON_FOUND )
    BISON_TARGET( SchemaAstGrammar schema-ast.y ${CMAKE_CURRENT_SOURCE_DIR}/schema-ast.cpp
        COMPILE_FLAGS "-Wno-other --no-lines -r state -v" )
    set( SchemaAstParser ${BISON_SchemaAstGrammar_OUTPUT_SOURCE} )
else()
    set( SchemaAstParser schema-ast.cpp )
endif()

add_library(schema STATIC
    AST.cpp
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

add_compile_definitions (__mod__="libs/schema")
