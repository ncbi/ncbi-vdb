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
    schema-ast.cpp
    schema-grammar.cpp
    schema-lex.c
    SchemaParser.cpp
    SchemaScanner.cpp
    Token.cpp
)

add_definitions (-D__mod__="libs/schema")