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

string GetVersionedName ( const AST_FQN& node )
{
    char buf[1024];
    node . GetVersionedName( buf, sizeof( buf ), true ); // #1 if version not specified
    return buf;
}

void pre_collectObjects( const ParseTree& node )
{
    auto& ast_node = dynamic_cast< const AST& >( node );
    switch( ast_node . GetTokenType() )
    {
    case PT_DATABASE:
        {   // database definition; TODO: support nested databases
            auto fqn = ToFQN( ast_node.GetChild(0) );
            assert( fqn );
            auto vers_name = GetVersionedName( *fqn );

            if ( g_si -> databases.find(vers_name) == g_si -> databases.end() )
            {
                g_si -> databases[ vers_name ] = SchemaInfo::Database( ast_node.GetChild(0)->GetLocation() );
            }
            else
            {
                throw logic_error( vers_name + ": database redefined" );
            }

            auto dad_node = ast_node.GetChild(1);
            if ( dad_node->GetTokenType() != PT_EMPTY )
            {   // add parent
                auto dad_fqn = ToFQN( dad_node );
                assert( fqn );
                auto dad_vers_name = GetVersionedName( *dad_fqn );
                g_si -> databases[ vers_name ].parent = dad_vers_name;
            }
            break;
        }
    case PT_TBLMEMBER:
        {   // database member table. record the type of the table, not its name in the DB
            // auto fqn = ToFQN( ast_node.GetChild(1) ); // type
            // assert( fqn );
            // g_si -> DbToTbl.NameMap::add( g_si -> activeDatabase, GetVersionedName( *fqn ) );
            // break;
        }
    case PT_TABLE:
        {   // table definition; TODO: support views
            // string name = GetFullName( ast_node.GetChild(0) );
            // auto fqn = ToFQN( ast_node.GetChild(0) );
            // assert( fqn );
            // auto vers_name = GetVersionedName( *fqn );
            // //cout << "table " << vers_name << endl;
            // g_si -> activeTable = vers_name;

            // assert( fqn );
            // g_si -> TblToCol.add( name, fqn->GetVersion(), ast_node.GetChild(0)->GetLocation() );
            // g_si -> TblToProd.add( name, fqn->GetVersion(), ast_node.GetChild(0)->GetLocation() );
            // auto plist = ast_node.GetChild(1);
            // if ( plist->GetTokenType() == PT_TABLEPARENTS )
            // { // add parents
            //     for ( uint32_t i = 0; i < plist->ChildrenCount(); ++i )
            //     {   //TODO: look for a definition with the correct version
            //         string dad = GetFullName( plist->GetChild(i) );
            //         auto dad_ver = ToFQN( plist->GetChild(i) )->GetVersion();
            //         if( g_si -> TblToCol.find(dad, dad_ver ) == g_si -> TblToCol.end() )
            //         {
            //             throw logic_error( string("table ") + name + " parent not found: " + dad );
            //         }
            //         const auto& d = g_si -> TblToCol.find( dad, dad_ver )->second;
            //         g_si -> TblToCol[vers_name].insert( d.begin(), d.end() );
            //         const auto& p = g_si -> TblToProd.find( dad, dad_ver )->second;
            //         g_si -> TblToProd[vers_name].insert( p.begin(), p.end() );
            //     }
            // }
            // else
            // {
            //     assert( plist->GetTokenType() == PT_EMPTY );
            // }
            break;
        }

    case PT_TYPEDCOL:
    case PT_TYPEDCOLEXPR:
        {   // column definition
            // assert( ast_node.ChildrenCount() >= 1 );
            // assert( ast_node.GetChild(0)->GetTokenType() == PT_IDENT );
            // string col_name = g_si -> activeTable + "." + GetFullName( ast_node.GetChild(0) );
            // // cout << "column " << col_name << endl;
            // g_si -> activeCol = col_name;
            // g_si -> ColToFn.add( col_name, ast_node.GetChild(0)->GetLocation() );
            // g_si -> ColToProd.add( col_name, ast_node.GetChild(0)->GetLocation() );
            // g_si -> TblToCol.NameMap::add( g_si -> activeTable, col_name );
            break;
        }

    case PT_FUNCDECL:
        {
            assert( ast_node.ChildrenCount() == 6 );
            string name = GetFullName( ast_node.GetChild(2) );
            auto fqn = ToFQN( ast_node.GetChild(2) );
            if( fqn )
            {
                name = GetVersionedName( *fqn );
            }
            else
            {
                assert(false);
            }

            if ( g_si -> functions.find(name) == g_si -> functions.end() )
            {
                g_si -> functions[ name ] = SchemaInfo::Function( ast_node.GetChild(2)->GetLocation() );
            }
            else
            {
                throw logic_error( name + ": function redefined" );
            }
            break;
        }

    case PT_FUNCEXPR:
        {   // function call: combine the name with the source location
            // string name = FunctionCallSignature( ast_node );
            // //cout << "function " << name << endl;
            // if ( !name.empty() )
            // {
            //     if ( !g_si -> activeCol.empty() )
            //     {
            //         g_si -> ColToFn.add( g_si -> activeCol, g_si -> activeTable + "." + name );
            //     }
            //     else if ( !g_si -> activeProd.empty() )
            //     {
            //         g_si -> ProdToFn.add( g_si -> activeProd, g_si -> activeTable + "." + name );
            //     }
            // }
            break;
        }
    case PT_PRODSTMT:
        {   // production
            // if ( ! g_si -> activeTable.empty() )
            // {
            //     string name = g_si -> activeTable + "." + GetFullName( ast_node.GetChild(1) );
            //     g_si -> activeProd = name;
            //     g_si -> ProdDefs.insert( name );
            //     g_si -> ProdToFn.add( name, ast_node.GetChild(1)->GetLocation() );
            //     g_si -> ProdToProd.add( name, ast_node.GetChild(1)->GetLocation() );
            //     //cout << "adding production " << name << " to " << g_si -> activeTable << endl;
            //     g_si -> TblToProd.NameMap::add( g_si -> activeTable, name );
            // }
            break;
        }

    case PT_IDENT:
        {   // use of an identifier
            //string name = ast_node.GetChild(0)->GetTokenValue();
            // string name = g_si -> activeTable + "." + GetFullName( ast_node.GetChild(0) );

            // if ( ! g_si -> activeProd.empty() )
            // {   // in a production
            //     //cout << g_si -> activeProd << " " << name << endl;
            //     if ( name != g_si -> activeProd )
            //     {
            //         g_si -> ProdToProd.add( g_si -> activeProd, name );

            //         auto p = g_si -> ProdToProd.find( name );
            //         if ( p != g_si -> ProdToProd.end() )
            //         {
            //             g_si -> ProdToProd[g_si -> activeProd].insert( p->second.begin(), p->second.end() );
            //         }
            //         p = g_si -> ProdToFn.find( name );
            //         if ( p != g_si -> ProdToFn.end() )
            //         {
            //             g_si -> ProdToFn[g_si -> activeProd].insert( p->second.begin(), p->second.end() );
            //         }
            //     }
            // }
            // else if ( ! g_si -> activeCol.empty() )
            // {   // in a column definition
            //     //cout << g_si -> activeProd << " " << name << endl;
            //     if ( name != g_si -> activeCol )
            //     {
            //         g_si -> ColToProd.add( g_si -> activeCol, name );

            //         auto p = g_si -> ProdToProd.find( name );
            //         if ( p != g_si -> ProdToProd.end() )
            //         {
            //             g_si -> ColToProd[g_si -> activeCol].insert( p->second.begin(), p->second.end() );
            //         }
            //         p = g_si -> ProdToFn.find( name );
            //         if ( p != g_si -> ProdToFn.end() )
            //         {
            //             g_si -> ColToFn[g_si -> activeCol].insert( p->second.begin(), p->second.end() );
            //         }
            //     }
            // }
            // else
            // {
            //     //cout << "ident " << name << endl;
            // }

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
            //cout << "end database " << GetFullName( ast_node.GetChild(0) ) << endl;
            // g_si -> activeDatabase.clear();
            break;
        }

    case PT_TABLE:
        {
            // g_si -> activeTable.clear();
            break;
        }

    case PT_TYPEDCOL:
    case PT_TYPEDCOLEXPR:
        {
            //cout << "end column " << GetFullName( ast_node.GetChild(0) ) << endl;
            // g_si -> activeCol.clear();
            break;
        }
    case PT_PRODSTMT:
        {
            // cout << "end production " << GetFullName( ast_node.GetChild(1) ) << endl;
            // g_si -> activeProd.clear();
            break;
        }

    default:
        break;
    }
}

void
SchemaInfo::populate( const ncbi::SchemaParser::AST & root )
{
    g_si = this;
    root . traverse( pre_collectObjects, post_collectObjects );
}