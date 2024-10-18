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
#include <klib/rc.h>

#include <ktst/unit_test.hpp> // TEST_CASE

#include "WVDB_Fixture.hpp"

using namespace std;

TEST_SUITE( LinkerTestSuite )

FIXTURE_TEST_CASE ( CallIntrinsic, WVDB_Fixture )
{
    const string schemaText =
"function < type T > T echo #1.0 < T val > ( * any row_len ) = vdb:echo;\n"
"table T #1 \n"
"{\n"
"    column ascii label = < ascii > echo < 'label' > ();\n"
"};\n"
"database db #1\n"
"{\n"
"    table T #1 tbl;\n"
"};\n"
;
    const char* TableName = "tbl";
    const char* ColumnName = "label";

    MakeDatabase ( GetName(), schemaText, "db" );

    {
        VCursor* cursor = CreateTable ( TableName );
        REQUIRE_RC ( VCursorOpen ( cursor ) );
        REQUIRE_RC ( VCursorCommit ( cursor ) );
        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
    {   // reopen
        const VCursor* cursor = OpenTable ( TableName );

        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        char buf[1024];
        uint32_t rowLen = 0;
        REQUIRE_RC( VCursorReadDirect ( cursor, 1, column_idx, 8, buf, sizeof ( buf ), & rowLen ) );
        REQUIRE_EQ( 5, (int)rowLen );
        REQUIRE_EQ( string("label"), string( buf, rowLen ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
}

static
rc_t CC echo_func(
                 void *Self,
                 const VXformInfo *info,
                 int64_t row_id,
                 VRowResult *rslt,
                 uint32_t argc,
                 const VRowData argv[]
)
{
    KDataBufferWhack(rslt->data);
    rslt->elem_count = 2;
    rslt->elem_bits = 8;
    KDataBufferResize ( rslt->data, 2 );
    ((char*)rslt->data->base)[0]='@';
    ((char*)rslt->data->base)[1]=0;
    return 0;
}

VTRANSFACT_IMPL ( user_echo, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    rslt->self = nullptr;
    rslt->whack = nullptr;
    rslt->variant = vftRow;
    rslt->u.rf = echo_func;
    return 0;
}

FIXTURE_TEST_CASE ( OverrideIntrinsic, WVDB_Fixture )
{   // insert our own version of vdb:echo

    VTRANSFACT_DECL ( user_echo );

    VLinkerIntFactory fact = {  user_echo, "vdb:echo" };

    REQUIRE_RC( VDBManagerAddFactories ( m_mgr,  &fact, 0 ) );

    const string schemaText =
"function < type T > T echo #1.0 < T val > ( * any row_len ) = vdb:echo;\n"
"table T #1 \n"
"{\n"
"    column ascii label = < ascii > echo < 'label' > ();\n"
"};\n"
"database db #1\n"
"{\n"
"    table T #1 tbl;\n"
"};\n"
;
    const char* TableName = "tbl";
    const char* ColumnName = "label";

    MakeDatabase ( GetName(), schemaText, "db" );

    {
        VCursor* cursor = CreateTable ( TableName );
        REQUIRE_RC ( VCursorOpen ( cursor ) );
        REQUIRE_RC ( VCursorCommit ( cursor ) );
        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }
    {   // reopen
        const VCursor* cursor = OpenTable ( TableName );

        uint32_t column_idx;
        REQUIRE_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        REQUIRE_RC ( VCursorOpen ( cursor ) );

        char buf[1024];
        uint32_t rowLen = 0;
        REQUIRE_RC( VCursorReadDirect ( cursor, 1, column_idx, 8, buf, sizeof ( buf ), & rowLen ) );
        REQUIRE_EQ( 2, (int)rowLen );
        REQUIRE_EQ( string("@"), string( buf, rowLen ) );

        REQUIRE_RC ( VCursorRelease ( cursor ) );
    }

}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "test-linker";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=LinkerTestSuite(argc, argv);
    return rc;
}

}
