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

#include <klib/printf.h>

// #include <vdb/xform.h>

#include <map>
#include <set>
#include <iostream>

namespace ncbi
{

struct SchemaInfo
{
    template <typename T>
    class NameMap : public std::map<std::string, T >
    {
    public:
        void addUnique( const std::string& key, const T& value ) // throw if exists
        {
            if ( this->find( key ) != this->end() )
            {
                throw std::logic_error( key + "is already defined" );
            }
            this->insert( make_pair( key, value ) );
        }
    };

    template <typename T>
    class VersionedNameMap : public NameMap<T>
    {
    public:
        void addUnique( const std::string & name, ver_t version, const T& value )
        {
            char buf[1024];
            string_printf ( buf, sizeof( buf ), nullptr, "%s#%V", name.c_str(), version );
            NameMap<T>::addUnique( buf, value ); // throws if already defined
            nameToVersions[name].insert(version);
        }

        typename NameMap<T>::const_iterator find(const std::string & name_vers) const
        {   // "name#version"
            return NameMap<T>::find( name_vers );
        }

        typename NameMap<T>::const_iterator find( const std::string & name, ver_t version ) const
        {   // find the version that fits the best
            const auto n = nameToVersions.find( name );
            if( n == nameToVersions.end() )
            {
                return this->end();
            }

            ver_t best_fit = findBestFit( n->second, version );
            if( best_fit == 0 )
            {
                return this->end();
            }

            char buf[1024];
            string_printf ( buf, sizeof( buf ), nullptr, "%s#%V", name.c_str(), best_fit );
            return NameMap<T>::find( std::string( buf ) );
        }

    private:
        ver_t findBestFit( const std::set<ver_t>& ver_set, ver_t version ) const;

        std::map< std::string, std::set<ver_t> > nameToVersions;
    };

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

    struct Production : public SchemaObject
    {
        std::set<std::string> id; // columns and productions in the right hand part of declaration
        std::set<std::string> calls; // function calls in the right hand part of declaration
    };

    struct Expression
    {   // right hand side of a column or production definition
        ncbi::SchemaParser::Token::Location location; // the column/production's location
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

    struct Definition
    {
        std::string owner; // table key
        bool is_column; // false = production

        Definition();
        Definition( const std::string& p_owner, bool p_is_column );

        bool empty() const { return owner.empty(); }
    };

    // tarverse the AST and populate the data structures
    SchemaInfo( const ncbi::SchemaParser::AST & root );

    // look for the definition of id (column or production) in the table or any of its ancestor
    Definition resolve( const std::string& t, const std::string& id ) const;

    //TODO: support views

    VersionedNameMap<Database> databases;
    std::string active_database;

    VersionedNameMap<Table> tables;
    std::string active_table;

    VersionedNameMap<Function> functions;
    Expression * active_expression = nullptr;

};

template<typename T>
ver_t
SchemaInfo::VersionedNameMap<T>::findBestFit( const std::set<ver_t>& ver_set, ver_t version ) const
{
    ver_t best_fit = 0;
    auto major = VersionGetMajor( version );
    auto minor = VersionGetMinor( version );
    auto release = VersionGetRelease( version );
    for ( auto i : ver_set )
    {
        if ( major == VersionGetMajor( i ) )
        {   // check minor & release
            auto i_minor = VersionGetMinor( i );
            auto i_release = VersionGetRelease( i );
            if ( minor == 0 && release == 0 ) // minor is unspecified
            {
                best_fit = i;
                break;
            }
            else if ( minor == i_minor )
            {   // check release
                if ( release == i_release )
                {   // exact match
                    best_fit = i;
                    break;
                }
                if ( release == 0 )
                {   // select the highest release
                    if ( best_fit == 0 )
                    {
                        best_fit = i;
                    }
                    else if ( i_release > VersionGetRelease( best_fit ) )
                    {
                        best_fit = i;
                    }
                }
            }
        }
    }
    return best_fit;
}

// collect a table's direct and indirect ancestors
std::set<std::string> TablesClosure( const SchemaInfo& si, const std::string& tbl );

// collect all function calls made directly or indirectly from the given column or production (id) referred to from tbl,
// using top_table as the context (i.e. root of inheritance hierarchy) for name resolution
std::set<std::string>
FunctionCallClosure( const SchemaInfo& si, const std::string& top_table, const std::string& tbl, const std::string& id );

}