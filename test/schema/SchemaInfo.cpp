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

#include "SchemaInfo.hpp"

using namespace ncbi::SchemaParser;
#include <ErrorReport.hpp>
#include <schema-grammar.hpp>

#include <sstream>

using namespace std;
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

void pre_collectObjects( const ParseTree& node )
{
    auto& ast_node = dynamic_cast< const AST& >( node );
    switch( ast_node . GetTokenType() )
    {
    case PT_DATABASE:
        {   // database definition; TODO: support nested databases
            const auto& name_node = *ast_node.GetChild(0);
            const auto& dad_node  = *ast_node.GetChild(1);

            auto vers_name = GetVersionedName( name_node );
            g_si -> databases.addUnique( vers_name, SchemaInfo::Database( name_node . GetLocation() ) );

            if ( dad_node . GetTokenType() != PT_EMPTY )
            {   // add parent
                auto dad_vers_name = GetVersionedName( dad_node );
                g_si -> databases[ vers_name ] . parent = dad_vers_name;
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
            const auto& name_node = *ast_node.GetChild(0);
            const auto& parents_node = *ast_node.GetChild(1);

            auto vers_name = GetVersionedName( name_node );
            g_si->tables.addUnique( vers_name, SchemaInfo::Table( name_node . GetLocation() ) );

            g_si -> active_table = vers_name;

            if ( parents_node . GetTokenType() == PT_TABLEPARENTS )
            { // add parents
                for ( uint32_t i = 0; i < parents_node . ChildrenCount(); ++i )
                {   //TODO: look for a definition with the correct version
                    auto dad_vers_name = GetVersionedName( *parents_node . GetChild(i) );
                    if ( g_si -> tables.find( dad_vers_name ) == g_si -> tables.end() )
                    {
                        throw logic_error( string("table ") + vers_name + " parent not found: " + dad_vers_name );
                    }
                    g_si -> tables[ vers_name ] . parents . insert( dad_vers_name );
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
            assert( ast_node.GetChild(0)->GetTokenType() == PT_IDENT );
            assert( ! g_si -> active_table.empty() );

            string col_name = GetFullName( ast_node.GetChild(0) );
            g_si -> tables[ g_si -> active_table ] . columns[ col_name ] = SchemaInfo::Expression();
            g_si -> active_expression = & g_si -> tables[ g_si -> active_table ] . columns[ col_name ];
            break;
        }

    case PT_FUNCDECL:
        {
            assert( ast_node.ChildrenCount() == 6 );
            const auto& name_node = *ast_node.GetChild(2);

            string vers_name = GetVersionedName( name_node );
            g_si -> functions . addUnique( vers_name, SchemaInfo::Function( name_node . GetLocation() ) );
            break;
        }

    case PT_FUNCEXPR:
        {   // function call: combine the name with the source location
            assert( g_si -> active_expression );
            assert( ast_node.ChildrenCount() == 4 );
            // 0:schema_parms_opt 1:fqn_opt_vers 2:factory_parms_opt 3:func_parms_opt
            auto fn_name = GetVersionedName( *ast_node.GetChild(1) );
            g_si -> active_expression -> calls . insert( fn_name );
            break;
        }
    case PT_PRODSTMT:
        {   // production
            assert ( ! g_si -> active_table.empty() );
            const auto& name_node = *ast_node.GetChild(1);

            string prod_name = GetFullName( &name_node );
            g_si -> tables[ g_si -> active_table ] . productions[ prod_name ] = SchemaInfo::Expression();
            g_si -> active_expression = & g_si -> tables[ g_si -> active_table ] . productions[ prod_name ];
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

template <typename T>
void
NameMap<T>::addUnique( const std::string& key, const T& value )
{
    if ( this->find( key ) != this->end() )
    {
        throw logic_error( key + "is alread defined" );
    }
    this->insert( make_pair( key, value ) );
}

void
SchemaInfo::populate( const ncbi::SchemaParser::AST & root )
{
    g_si = this;
    root . traverse( pre_collectObjects, post_collectObjects );
}