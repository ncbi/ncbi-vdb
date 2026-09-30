/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

/**
* Schema objects and interdependencies
*/

#include <AST.hpp>

// #include "AST_Fixture.hpp"

// #include <ktst/unit_test.hpp>

// #include <kfc/defs.h>

// #include <klib/printf.h>

// #include <vdb/xform.h>

#include <map>
#include <set>
#include <iostream>

template <typename T>
class NameMap : public std::map<std::string, T >
{
    public:
        void addUnique( const std::string& key, const T& value ); // throw if exists

        void print( std::ostream& out ) const
        {
            for ( auto i : *this )
            {
                out << i.first << ": " << std::endl;
            }
        }
};

template <typename T>
class VersionedNameMap : public NameMap<T>
{
};

struct SchemaInfo
{
    class SchemaObject
    {
    public:
        SchemaObject();
        SchemaObject( const ncbi::SchemaParser::Token::Location& p_loc );

        const ncbi::SchemaParser::Token::Location& getLocation() const { return m_location; }

    private:
        ncbi::SchemaParser::Token::Location m_location;
    };

    class Function : public SchemaObject
    {
    public:
        Function();
        Function( const ncbi::SchemaParser::Token::Location& p_loc );
    };

    class Production : public SchemaObject
    {
        std::set<std::string> id; // columns and productions in the right hand part of declaration
        std::set<std::string> calls; // function calls in the right hand part of declaration
    };

    class Expression
    {   // right hand side of a column or production definition
    public:
        std::set<std::string> ids;   // columns and/or productions directly mentioned in the expression
        std::set<std::string> calls; // function calls directly made in the expression
    };

    class Table : public SchemaObject
    {
    public:
        Table();
        Table( const ncbi::SchemaParser::Token::Location& p_loc );

        std::set<std::string> parents;
        std::map<std::string, Expression> columns;  // defined in this table
        std::map<std::string, Expression> productions;  // defined in this table
    };

    class Database : public SchemaObject
    {
    public:
        Database();
        Database( const ncbi::SchemaParser::Token::Location& p_loc );

        std::string parent;
        std::set<std::string> tables;
    };

    //TODO: support views

    VersionedNameMap<Database> databases;
    std::string active_database;

    VersionedNameMap<Table> tables;
    std::string active_table;

    VersionedNameMap<Function> functions;
    Expression * active_expression = nullptr;

    void populate( const ncbi::SchemaParser::AST & root );

};