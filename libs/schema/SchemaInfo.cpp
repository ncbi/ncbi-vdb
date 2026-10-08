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

#include <schema/SchemaInfo.hpp>

#include <schema/AST.hpp>

using namespace ncbi::SchemaParser;
#include "ErrorReport.hpp"
#include "schema-grammar.hpp"

#include <sstream>

using namespace std;
using namespace ncbi;
using namespace ncbi::SchemaParser;

//TODO: make thread safe
static SchemaInfo * g_si;

SchemaInfo::SchemaObject::SchemaObject()
{
}

SchemaInfo::SchemaObject::SchemaObject( const ncbi::SchemaParser::Token::Location& p_loc )
: m_location( p_loc )
{
}


SchemaInfo::Function::Function()
{
}

SchemaInfo::Function::Function( const ncbi::SchemaParser::Token::Location& p_loc )
: SchemaObject( p_loc )
{
}

SchemaInfo::Database::Database()
{
}

SchemaInfo::Database::Database( const ncbi::SchemaParser::Token::Location& p_loc )
: SchemaObject( p_loc )
{
}

SchemaInfo::Definition::Definition()
: owner(""), is_column(false)
{
}

SchemaInfo::Definition::Definition( const string & p_owner, bool p_is_column )
: owner(p_owner), is_column(p_is_column)
{
}


SchemaInfo::Table::Table()
{
}

SchemaInfo::Table::Table( const ncbi::SchemaParser::Token::Location& p_loc )
: SchemaObject( p_loc )
{
}

string GetFullName ( const AST* node )
{
    switch( node -> GetTokenType() )
    {
    case PT_IDENT:
        {
            auto fqn = ToFQN( node );
            if ( fqn )
            {
                char buf[1024];
                fqn -> GetFullName( buf, sizeof( buf ) );
                return buf;
            }
            else
            {
                assert( node->ChildrenCount() == 1 );
                auto id = node->GetChild(0);
                assert( id->GetTokenType() == IDENTIFIER_1_0 );
                return id->GetTokenValue();
            }
        }
    case IDENTIFIER_1_0:
        {
            return node->GetTokenValue();
        }
    default:
        {
            ostringstream s;
            s << "GetFullName(): unexpected tag " << node -> GetTokenType();
            throw logic_error( s.str() );
        }
    }
    return string();
}

string GetVersionedName ( const AST& node )
{
    auto fqn = ToFQN( &node );
    assert( fqn );
    char buf[1024];
    fqn -> GetVersionedName( buf, sizeof( buf ), true ); // #1 if version not specified
    return string( buf );
}

string FunctionCallSignature( const AST& node )
{
    assert( node.GetTokenType() == PT_FUNCEXPR );
    assert( node.ChildrenCount() == 4 );
    // 0:schema_parms_opt 1:fqn_opt_vers 2:factory_parms_opt 3:func_parms_opt
    auto fqn = ToFQN( node.GetChild(1) );
    assert( fqn );
    string ret = GetVersionedName( *fqn );

    ret += "(";

    auto func_parms = node.GetChild(3);
    size_t fp_count = func_parms->ChildrenCount();
    for ( size_t i = 0; i < fp_count; ++i )
    {
        if ( i > 0 )
        {
            ret += ",";
        }
        // allowed tags: PT_AT, PHYSICAL_IDENTIFIER_1_0, PT_CAST, PT_IDENT, PT_MEMBEREXPR
        auto param = func_parms->GetChild( i );
        switch ( param->GetTokenType() )
        {
        case PT_IDENT:
            ret += GetFullName( param->GetChild(0) );
            break;
        case '@':
            ret += "@";
            break;
        case PHYSICAL_IDENTIFIER_1_0:
            ret += param->GetTokenValue();
            break;
        case PT_CASTEXPR:
        case PT_MEMBEREXPR:
        default:
            assert(false);
        }
    }
    return ret + ")";
}


// traverse an expression and collect all identifiers and function calls
SchemaInfo::Expression * g_expression = nullptr;
void pre_collectIds( const ParseTree& node )
{
    assert( g_expression );

    auto& ast_node = dynamic_cast< const AST& >( node );
    switch( ast_node . GetTokenType() )
    {
    case PT_IDENT:
        {
            auto fqn = ToFQN( &ast_node );
            if ( fqn )
            {
                char buf[1024];
                fqn -> GetFullName( buf, sizeof( buf ) );
                g_expression -> ids . insert( buf );
            }
            else
            {
                assert( ast_node.ChildrenCount() == 1 );
                auto id = ast_node.GetChild(0);
                if( id->GetTokenType() == IDENTIFIER_1_0 )
                {
                    g_expression -> ids . insert( id->GetTokenValue() );
                }
            }
            break;
        }

    case IDENTIFIER_1_0:
        {
            g_expression -> ids . insert( ast_node . GetTokenValue() );
            break;
        }

    case PT_FUNCEXPR:
        {   // function call
            assert( g_expression );
            assert( ast_node.ChildrenCount() == 4 );
            // 0:schema_parms_opt 1:fqn_opt_vers 2:factory_parms_opt 3:func_parms_opt
            g_expression -> calls . insert( FunctionCallSignature( ast_node ) );
            break;
        }

    default:
        break;
    }
}

void pre_collectObjects( const ParseTree& node )
{
    auto& ast_node = dynamic_cast< const AST& >( node );
    switch( ast_node . GetTokenType() )
    {
    case PT_DATABASE:
        {   // database definition; TODO: support nested databases
            const auto& name_node = *ToFQN( ast_node.GetChild(0) );
            const auto& dad_node  = *ast_node.GetChild(1);

            g_si -> databases.addUnique(
                GetFullName( &name_node ),
                name_node.GetVersion(),
                SchemaInfo::Database( name_node . GetLocation() ) );

            auto vers_name = GetVersionedName( name_node );
            if ( dad_node . GetTokenType() != PT_EMPTY )
            {   // add parent
                const auto& dad_fqn = *ToFQN( & dad_node );
                auto best_dad = g_si->databases.find( GetFullName( &dad_fqn ), dad_fqn.GetVersion() );
                if ( best_dad == g_si -> databases.end() )
                {
                    throw logic_error( string("database ") + vers_name + " parent not found: " + GetVersionedName( dad_node ) );
                }
                g_si -> databases[ vers_name ] . parent = best_dad->first;
            }

            g_si -> active_database = vers_name;

            break;
        }
    case PT_TBLMEMBER:
        {   // database member table. record the type of the table, not its name in the DB
            assert( ! g_si -> active_database.empty() );
            g_si -> databases[ g_si -> active_database ] .tables.insert( GetVersionedName( *ast_node.GetChild(1) ) );
            break;
        }
    case PT_TABLE:
        {   // table definition
            const auto& name_node = *ToFQN( ast_node.GetChild(0) );
            const auto& parents_node = *ast_node.GetChild(1);

            g_si->tables.addUnique(
                GetFullName( &name_node ),
                name_node.GetVersion(),
                SchemaInfo::Table( name_node . GetLocation() ) );

            auto vers_name = GetVersionedName( name_node );
            g_si -> active_table = vers_name;

            if ( parents_node . GetTokenType() == PT_TABLEPARENTS )
            { // add parents
                for ( uint32_t i = 0; i < parents_node . ChildrenCount(); ++i )
                {   // look for a definition with the correct version
                    const auto& dad_node = *ToFQN( parents_node . GetChild(i) );
                    auto best_dad = g_si->tables.find( GetFullName( &dad_node ), dad_node.GetVersion() );
                    if ( best_dad == g_si -> tables.end() )
                    {
                        throw logic_error( string("table ") + vers_name + " parent not found: " + GetVersionedName( dad_node ) );
                    }
                    g_si -> tables[ vers_name ] . parents . insert( best_dad->first );
                }
            }
            else
            {
                assert( parents_node . GetTokenType() == PT_EMPTY );
            }
            break;
        }

    case PT_TYPEDCOL:
    case PT_TYPEDCOLEXPR:
        {   // column definition
            assert( ast_node.ChildrenCount() >= 1 );
            const auto& name_node = *ast_node.GetChild(0);
            assert( name_node.GetTokenType() == PT_IDENT );
            assert( ! g_si -> active_table.empty() );

            // sweep identifers from the initialization expression, if given
            SchemaInfo::Expression expr;
            expr.location = name_node.GetLocation();
            if ( ast_node.ChildrenCount() == 2 )
            {
                g_expression = & expr;
                ast_node . GetChild( 1 ) -> traverse( pre_collectIds, nullptr );
                g_expression = nullptr;
            }

            g_si -> tables[ g_si -> active_table ] . columns[ GetFullName( &name_node ) ] = expr;

            break;
        }

    case PT_FUNCDECL:
        {
            assert( ast_node.ChildrenCount() == 6 );
            const auto& name_node = *ToFQN(ast_node.GetChild(2));

            g_si -> functions . addUnique(
                GetFullName( &name_node ),
                name_node.GetVersion(),
                SchemaInfo::Function( name_node . GetLocation() ) );
            break;
        }

    case PT_PRODSTMT:
        {   // production
            if ( ! g_si -> active_table.empty() )
            {
                const auto& name_node = *ast_node.GetChild( 1 );

                // sweep identifers from the right hand side
                SchemaInfo::Expression expr;
                if ( ast_node.ChildrenCount() >= 3 )
                {
                    const auto& rhs_node = *ast_node.GetChild( 2 );
                    g_expression = & expr;
                    rhs_node . traverse( pre_collectIds, nullptr );
                    g_expression = nullptr;
                }

                string prod_name = GetFullName( &name_node );
                g_si -> tables[ g_si -> active_table ] . productions[ prod_name ] = expr;
            }
            break;
        }

    case PT_IDENT:
        {   // use of an identifier
            if( g_si -> active_expression )
            {
                string name = ast_node.GetChild(0)->GetTokenValue();
                g_si -> active_expression -> ids . insert( name );
                cout<< "PT_IDENT(" << name << ")" << endl;
            }
            break;
        }

    default:
        break;
    }
}

void post_collectObjects( const ParseTree& node )
{
    auto& ast_node = dynamic_cast< const AST& >( node );
    switch( ast_node . GetTokenType() )
    {
    case PT_DATABASE:
        {
            g_si -> active_database.clear();
            break;
        }

    case PT_TABLE:
        {
            g_si -> active_table.clear();
            break;
        }

    case PT_TYPEDCOL:
    case PT_TYPEDCOLEXPR:
        {
            g_si -> active_expression = nullptr;
            break;
        }
    case PT_PRODSTMT:
        {
            g_si -> active_expression = nullptr;
            break;
        }

    default:
        break;
    }
}

SchemaInfo::SchemaInfo()
{
}

SchemaInfo::SchemaInfo( const ncbi::SchemaParser::AST & root )
{
    g_si = this;
    root . traverse( pre_collectObjects, post_collectObjects );
}

SchemaInfo::Definition
SchemaInfo::resolve( const string& p_tbl, const string& p_id ) const
{
    auto tbl_it = tables.find( p_tbl );
    if ( tbl_it != tables.end() )
    {
        const auto& t = tbl_it->second;
        auto it = t.columns.find( p_id );
        if ( it != t.columns.end() )
        {
            return Definition( tbl_it->first, true );
        }
        it = t.productions.find( p_id );
        if ( it != t.productions.end() )
        {
            return Definition( tbl_it->first, false );
        }

        // look up the name in the ancestors
        for ( auto p : t.parents )
        {
            auto def = resolve( p, p_id );
            if ( !def.empty() )
            {
                return def;
            }
        }
    }

    return Definition();
}

std::set<std::string>
ncbi::DatabaseClosure( const SchemaInfo& si, const std::string& db )
{
    auto db_it = si.databases.find( db );
    assert ( db_it != si.databases.end() );

    set<string> ret;
    ret.insert( db );

    const auto& p = db_it->second.parent;

    if ( ! p.empty() )
    {
        auto inherited = DatabaseClosure( si, p );
        ret.insert( inherited.begin(), inherited.end() );
    }

    return ret;
}

set<string>
ncbi::TablesClosure( const SchemaInfo& si, const string& tbl )
{
    auto tbl_it = si.tables.find( tbl );
    assert ( tbl_it != si.tables.end() );

    set<string> ret;
    ret.insert( tbl );

    const auto& t = tbl_it->second;
    if ( t.parents.size() > 0 )
    {
        for (auto c : t.parents)
        {
            auto inherited = TablesClosure( si, c );
            ret.insert( inherited.begin(), inherited.end() );
        }
    }

    return ret;
}

set<string>
ncbi::FunctionCallClosure( const SchemaInfo& si, const string& top_table, const string& tbl, const string& id )
{
//cout << "FunctionCallClosure(" << top_table << ", " << tbl << ", " << id << ")" << endl;

    set<string> ret;

    auto def = si.resolve( top_table, id );
    if ( ! def.empty() )
    {
        auto tbl_it = si.tables.find( def.owner );
        assert ( tbl_it != si.tables.end() );

        SchemaInfo::Expression expr;

        if (def.is_column)
        {
            auto it = tbl_it->second.columns.find( id );
            assert ( it != tbl_it->second.columns.end() );
            expr = it->second;
        }
        else // production
        {
            auto it = tbl_it->second.productions.find( id );
            assert( it != tbl_it->second.productions.end() );
            expr = it->second;
        }

        // direct calls from columns' right hand side expressions
        for ( auto c : expr.calls )
        {
            ret.insert( c );
        }
        // resolve ids and dive into productions
        for ( auto i : expr.ids )
        {
            auto def = si.resolve( top_table, i );
            if ( !def.empty() && ! def.is_column ) // a production defined sowehere in top_table's inheritance hierarchy
            {
                auto c = FunctionCallClosure( si, top_table, def.owner, i );
                ret.insert( c.begin(), c.end() );
            }
        }
    }

    return ret;
}