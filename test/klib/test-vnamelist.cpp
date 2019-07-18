// ===========================================================================
//
//                            PUBLIC DOMAIN NOTICE
//               National Center for Biotechnology Information
//
//  This software/database is a "United States Government Work" under the
//  terms of the United States Copyright Act.  It was written as part of
//  the author's official duties as a United States Government employee and
//  thus cannot be copyrighted.  This software/database is freely available
//  to the public for use. The National Library of Medicine and the U.S.
//  Government have not placed any restriction on its use or reproduction.
//
//  Although all reasonable efforts have been taken to ensure the accuracy
//  and reliability of the software and data, the NLM and the U.S.
//  Government do not and cannot warrant the performance or results that
//  may be obtained by using this software or data. The NLM and the U.S.
//  Government disclaim all warranties, express or implied, including
//  warranties of performance, merchantability or fitness for any particular
//  purpose.
//
//  Please cite the author in any work or product based on this material.
//
// ===========================================================================

#include <vdb/manager.h> // VDBManager
#include <vdb/vdb-priv.h>

#include <ktst/unit_test.hpp> // TEST_CASE
#include <klib/text.h> 
#include <klib/out.h> 
#include <klib/namelist.h> 

#include <sysalloc.h>
#include <cstdlib>
#include <stdexcept>

using namespace std;

TEST_SUITE( T_VNamelist )


TEST_CASE( VNamelist_1 )
{
    VNamelist * list;
    rc_t rc = VNamelistMake ( &list, 10 );
    if ( rc != 0 )
        FAIL( "FAIL: VNamelistMake( 10 ) failed" );

    rc = VNamelistRelease ( list );
    if ( rc != 0 )
        FAIL( "FAIL: VNamelistRelease() failed" );
}

struct on_part_ctx
{
    const char ** v;
    int idx;
};

static rc_t CC on_part( const String * part, void * data )
{
    rc_t rc = 0;
    struct on_part_ctx * ctx = ( struct on_part_ctx * ) data;
    std::string p = std::string( part->addr, part->size );
    std::string c = std::string( ctx->v[ ctx->idx ] );
    if ( p != c ) rc = -1;
    ctx->idx++;
    return rc;
}


rc_t test_String( const char * to_test, const char ** v, int count )
{
    String s;
    struct on_part_ctx ctx;
    ctx.v = v;
    ctx.idx = 0;
    
    StringInitCString( &s, to_test );
    rc_t rc = foreach_String_part( &s, ':', on_part, &ctx );
    if ( rc == 0 && ctx.idx != count ) rc = -1;
    return rc;
}


rc_t test_pchar( const char * to_test, const char ** v, int count )
{
    struct on_part_ctx ctx;
    ctx.v = v;
    ctx.idx = 0;
    
    rc_t rc = foreach_Str_part( to_test, ':', on_part, &ctx );
    if ( rc == 0 && ctx.idx != count ) rc = -1;
    return rc;
}

static const char * s1 = "string:with:colons";
static const char * t1[] = { "string", "with", "colons" };
static const int n1 = 3;

static const char * s2 = "just a string";
static const char * t2[] = { "just a string" };
static const int n2 = 1;

static const char * s3 = "with::multiple::colons";
static const char * t3[] = { "with", "", "multiple", "", "colons" };
static const int n3 = 5;

static const char * s4 = "::leading:colons";
static const char * t4[] = { "", "", "leading", "colons" };
static const int n4 = 4;

static const char * s5 = "trailing:colons::";
static const char * t5[] = { "trailing", "colons", "", "" };
static const int n5 = 4;

TEST_CASE( StringSplit )
{
    std::cout << "testing String-splitting by callback" << std::endl;
    
    rc_t rc = test_String( s1, t1, n1 );
    if ( rc != 0 )
        FAIL( "FAIL: foreach_String_part( #1 ) failed" );
    rc = test_pchar( s1, t1, n1 );
    if ( rc != 0 )
        FAIL( "FAIL: foreach_Str_part( #1 ) failed" );

    rc = test_String( s2, t2, n2 );
    if ( rc != 0 )
        FAIL( "FAIL: foreach_String_part( #2 ) failed" );
    rc = test_pchar( s2, t2, n2 );
    if ( rc != 0 )
        FAIL( "FAIL: foreach_Str_part( #2 ) failed" );

    rc = test_String( s3, t3, n3 );
    if ( rc != 0 )
        FAIL( "FAIL: foreach_String_part( #3) failed" );
    rc = test_pchar( s3, t3, n3 );
    if ( rc != 0 )
        FAIL( "FAIL: foreach_Str_part( #3 ) failed" );

    rc = test_String( s4, t4, n4 );
    if ( rc != 0 )
        FAIL( "FAIL: foreach_String_part( #4 ) failed" );
    rc = test_pchar( s4, t4, n4 );
    if ( rc != 0 )
        FAIL( "FAIL: foreach_Str_part( #4 ) failed" );

    rc = test_String( s5, t5, n5 );
    if ( rc != 0 )
        FAIL( "FAIL: foreach_String_part( #5 ) failed" );
    rc = test_pchar( s5, t5, n5 );
    if ( rc != 0 )
        FAIL( "FAIL: foreach_Str_part( #5 ) failed" );
}


rc_t insert_String( VNamelist * list, const char * to_test )
{
    String s;
    StringInitCString( &s, to_test );
    return VNamelistSplitString ( list, &s, ':' );
}


rc_t insert_pchar( VNamelist * list, const char * to_test )
{
    return VNamelistSplitStr ( list, to_test, ':' );
}


rc_t check_list( const VNamelist * list, const char ** v, uint32_t count )
{
    rc_t rc = 0;
    uint32_t idx;
    for ( idx = 0; rc == 0 && idx < count; ++idx )
    {
        const char * item;
        rc = VNameListGet ( list, idx, &item );
        if ( rc == 0 )
        {
            std::string s_item = std::string( item );
            std::string s_cmp  = std::string( v[ idx ] );
            if ( s_item.compare( s_cmp ) != 0 )
                rc = -1;
        }
        if ( rc == 0 )
        {
            uint32_t lc;
            rc = VNameListCount ( list, &lc );
            if ( rc == 0 && lc != count )
                rc = -1;
        }
    }
    return rc;
}


rc_t test_split_string( const char * to_test, const char ** v, uint32_t count )
{
    VNamelist * list;
    rc_t rc = VNamelistMake ( &list, 10 );
    if ( rc == 0 )
    {
        rc = insert_String( list, to_test );
        if ( rc == 0 )
            rc = check_list( list, v, count );
        if ( rc == 0 )
            rc = VNamelistRemoveAll( list );

        if ( rc == 0 )
            rc = insert_pchar( list, to_test );
        if ( rc == 0 )
            rc = check_list( list, v, count );

        VNamelistRelease ( list );
    }
    return rc;
}

TEST_CASE( SplitIntoVNamelist )
{
    std::cout << "testing String-splitting into existing VNamelist " << std::endl;
    
    rc_t rc = test_split_string( s1, t1, n1 );
    if ( rc != 0 )
        FAIL( "FAIL: test_split_string( #1 )" );

    rc = test_split_string( s2, t2, n2 );
    if ( rc != 0 )
        FAIL( "FAIL: test_split_string( #2 )" );

    rc = test_split_string( s3, t3, n3 );
    if ( rc != 0 )
        FAIL( "FAIL: test_split_string( #3 )" );

    rc = test_split_string( s4, t4, n4 );
    if ( rc != 0 )
        FAIL( "FAIL: test_split_string( #4 )" );

    rc = test_split_string( s5, t5, n5 );
    if ( rc != 0 )
        FAIL( "FAIL: test_split_string( #5 )" );
        
}


rc_t test_make_from_string( const char * to_test, const char ** v, uint32_t count )
{
    String s;
    StringInitCString( &s, to_test );

    VNamelist * list;
    rc_t rc = VNamelistFromString( &list, &s, ':' );
    if ( rc == 0 )
    {
        rc = check_list( list, v, count );
        VNamelistRelease ( list );
    }
    
    if ( rc == 0 )
    {
        rc = VNamelistFromStr( &list, to_test, ':' );
        if ( rc == 0 )
        {
            rc = check_list( list, v, count );
            VNamelistRelease ( list );
        }
    }
    return rc;
}


TEST_CASE( MakeVNamelistFromString )
{
    std::cout << "testing String-splitting into new VNamelist " << std::endl;
    
    rc_t rc = test_make_from_string( s1, t1, n1 );
    if ( rc != 0 )
        FAIL( "FAIL: test_make_from_string( #1 )" );

    rc = test_make_from_string( s2, t2, n2 );
    if ( rc != 0 )
        FAIL( "FAIL: test_make_from_string( #2 )" );

    rc = test_make_from_string( s3, t3, n3 );
    if ( rc != 0 )
        FAIL( "FAIL: test_make_from_string( #3 )" );

    rc = test_make_from_string( s4, t4, n4 );
    if ( rc != 0 )
        FAIL( "FAIL: test_make_from_string( #4 )" );

    rc = test_make_from_string( s5, t5, n5 );
    if ( rc != 0 )
        FAIL( "FAIL: test_make_from_string( #5 )" );
}

rc_t split_join_and_check( const char * to_test )
{
    VNamelist * list;
    rc_t rc = VNamelistFromStr( &list, to_test, ':' );
    if ( rc == 0 )
    {
        const String * joined;
        rc = VNamelistJoin( list, ':', &joined );
        if ( rc != 0 )
            FAIL( "FAIL: VNamelistJoin()" );
        else
        {
            String S2;
            StringInitCString( &S2, to_test );
            if ( !StringEqual( joined, &S2 ) )
                rc = -1;
            StringWhack ( joined );
        }
        VNamelistRelease ( list );
    }
    return rc;
}

TEST_CASE( VNamelistJoining )
{
    std::cout << "testing joining VNamelist into one String" << std::endl;

    rc_t rc = split_join_and_check( s1 );
    if ( rc != 0 )
        FAIL( "FAIL: split_join_and_check( #1 )" );

    rc = split_join_and_check( s2 );
    if ( rc != 0 )
        FAIL( "FAIL: split_join_and_check( #2 )" );

    rc = split_join_and_check( s3 );
    if ( rc != 0 )
        FAIL( "FAIL: split_join_and_check( #3 )" );

    rc = split_join_and_check( s4 );
    if ( rc != 0 )
        FAIL( "FAIL: split_join_and_check( #4 )" );

    rc = split_join_and_check( s5 );
    if ( rc != 0 )
        FAIL( "FAIL: split_join_and_check( #5 )" );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>

ver_t CC KAppVersion ( void ) { return 0x1000000; }
rc_t CC UsageSummary ( const char * progname ) { return 0; }
rc_t CC Usage ( const Args * args ) { return 0; }
const char UsageDefaultName[] = "test-VDB-3060";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc = T_VNamelist( argc, argv );
    return rc;
}

}
