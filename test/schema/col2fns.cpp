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
* Unit tests for schema AST traversal
*/

#include "AST_Fixture.hpp"

#include "SchemaInfo.hpp"

#include <kapp/vdbapp.h>

using namespace std;
using namespace ncbi;

static
ver_t
MakeVer( uint8_t maj, uint8_t min = 0, uint8_t rel = 0 )
{
    return VTRANSVERS( maj, min, rel );
}

set<string>
FnWhiteList = {
    "NCBI:align:get_mate_align_id#1",
    "NCBI:align:ref_restore_read#1",
    "NCBI:align:cigar#1",
    "NCBI:align:cigar#2",
    "NCBI:align:edit_distance#1",
    "NCBI:align:edit_distance#2",
    "NCBI:align:generate_has_mismatch#1",
    "NCBI:align:generate_mismatch#1",
    "NCBI:align:get_left_soft_clip#2",
    "NCBI:align:get_clipped_cigar#2",
    "NCBI:align:get_ref_len#1",
    "NCBI:align:clip#2",
    "NCBI:align:ref_sub_select#1",
    "NCBI:align:seq_restore_read#1",
    "NCBI:align:align_restore_read#1",
    "NCBI:align:project_from_sequence#1",
    "NCBI:align:seq_construct_read#1",
    "NCBI:align:ref_name#1",
    "NCBI:fp_extend#1",
    "NCBI:SRA:extract_name_fmt#1",
    "NCBI:dna_from_color#1",
    "NCBI:SRA:bio_end#1",
    "outlier_encode#1",
    "strtonum#1",
    "sprintf#1",
    "rldecode#1",
    "checksum#1",
    "map#1",
    "rlencode#1",
};

//////////////////////////////////////////// Main

// collect a table's direct and indirect ancestors
void
TablesClosure( SchemaInfo si, string tbl, set<string>& closure )
{
    auto tbl_it = si.tables.find( tbl );
    assert ( tbl_it != si.tables.end() );

    const auto& t = tbl_it->second;
    if ( t.parents.size() > 0 )
    {
        for (auto c : t.parents)
        {
            closure.insert( c );
            TablesClosure( si, c, closure );
        }
    }
}

// collect all function calls made directly or indirectly from the given column or production
void
FunctionCallClosure( const SchemaInfo& si, const string& top_table, const string& tbl, string id, set<string>& closure )
{
    auto tbl_it = si.tables.find( top_table );
    assert ( tbl_it != si.tables.end() );

    auto col_it = tbl_it->second.columns.find( id );
    if ( col_it != tbl_it->second.columns.end() )
    {
        col_it = tbl_it->second.productions.find( id );
        assert( col_it != tbl_it->second.productions.end() );
    }

    const SchemaInfo::Expression& expr = col_it->second;
    // direct calls from columns' right hand side expressions
    for ( auto c : expr.calls )
    {
        closure.insert( c );
    }
    // resolve ids and dive into productions
    for ( auto i : expr.ids )
    {
        auto def = si.resolve( tbl_it->first, i );
        if ( !def.empty() && ! def.is_column ) // a production defined sowehere in top_table's inheritance hierarchy
        {
            FunctionCallClosure( si, top_table, def.owner, i, closure );
            closure.insert( i );
        }
    }
}

int main( int argc, char *argv [] )
{
    VDB::Application app( argc, argv, "" );

    //const string DB = "NCBI:align:db:alignment_unsorted#2";
    const string Tbl = "NCBI:align:tbl:seq#2";
    const string Col = "READ";
    AST_Fixture f;
    AST * root = f.MakeAst( "version 2; include 'align/align.vschema';" );

    SchemaInfo si( *root );

    auto tbl_it = si.tables.find( Tbl );
    if ( tbl_it == si.tables.end() )
    {
        cout << "Table " << Tbl << " is not found " << endl;
        return 1;
    }

    const auto& tbl = tbl_it->second;
    cout << "Table " << Tbl << "(" << LocationToString( tbl.getLocation() ) << "):" << endl; //TODO: location
    set<string> t_closure;

    TablesClosure( si, Tbl, t_closure );
    cout << "   Parent(s):" << endl;
    for ( auto t : t_closure )
    {
        if ( t != Tbl )
        {
            cout << "      " << t << "(" << LocationToString( si.tables.at(t).getLocation() ) << ")" << endl;
        }
    }
    // add the table itself
    t_closure.insert( Tbl );

    cout << "   Column(s):" << endl;
    for ( auto t : t_closure )
    {
        const auto& tbl = si.tables.find( t );
        for (auto c : tbl->second.columns)
        {
            if ( Col.empty() || c.first == Col )
            {
                cout << "      " << t << "." << c.first << "(" << LocationToString( c.second.location ) << ")"<< endl;
                set<string> calls;
                FunctionCallClosure( si, Tbl, Tbl, c.first, calls );
                for ( auto call : calls )
                {
                    cout << "         " << call << endl;
                }
            }
        }
    }

}

