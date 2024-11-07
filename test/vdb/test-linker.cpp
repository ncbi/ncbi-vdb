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

#include "WVDB_Fixture.hpp"

#include <bitstr.h>

using namespace std;

TEST_SUITE( LinkerTestSuite )

class TestLinkerFicture : public WVDB_Fixture
{
public:
    const char* TableName = "tbl";
    const char* ColumnName = "label";

    void MakeEmptyTable( const string & caseName, const string & schemaText )
    {
        MakeDatabase ( caseName, schemaText, "db" );
        VCursor* cursor = CreateTable ( TableName );
        THROW_ON_RC ( VCursorOpen ( cursor ) );
        THROW_ON_RC ( VCursorCommit ( cursor ) );
        THROW_ON_RC ( VCursorRelease ( cursor ) );
    }

    string ReadRow()
    {
        const VCursor* cursor = OpenTable ( TableName );

        uint32_t column_idx;
        THROW_ON_RC ( VCursorAddColumn ( cursor, & column_idx, ColumnName ) );
        THROW_ON_RC ( VCursorOpen ( cursor ) );

        char buf[1024];
        uint32_t rowLen = 0;
        THROW_ON_RC( VCursorReadDirect ( cursor, 1, column_idx, 8, buf, sizeof ( buf ), & rowLen ) );

        THROW_ON_RC ( VCursorRelease ( cursor ) );

        return string( buf, rowLen );
    }
};

FIXTURE_TEST_CASE ( CallIntrinsic, TestLinkerFicture )
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

    MakeEmptyTable ( GetName(), schemaText );
    REQUIRE_EQ( string("label"), ReadRow() );
}

static
rc_t CC newecho_func(
                 void *Self,
                 const VXformInfo *info,
                 int64_t row_id,
                 VRowResult *rslt,
                 uint32_t argc,
                 const VRowData argv[]
)
{
    rc_t rc = KDataBufferResize ( rslt->data, 2 );
    if ( rc == 0 )
    {
        ((char*)rslt->data->base)[0]='@';
        ((char*)rslt->data->base)[1]='#';
        rslt -> elem_count = 2;
    }
    return rc;
}

struct self_t {
    KDataBuffer val;
    bitsz_t csize;
    bitsz_t dsize;
    int count;
};

static void CC self_free( void *Self ) {
    struct self_t *self = (struct self_t *)Self;

    KDataBufferWhack(&self->val);
    free(self);
}

rc_t newecho_row_0 ( const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp )
{
    rc_t rc;
    struct self_t *self = (struct self_t *) malloc ( sizeof *self );
    if ( self == NULL )
        return RC(rcXF, rcFunction, rcConstructing, rcMemory, rcExhausted);

    self->dsize = VTypedescSizeof ( & cp->argv[0].desc );
    self->csize = self->dsize * cp->argv[0].count;
    self->count = 1;

    rc = KDataBufferMake(&self->val, self->dsize, cp->argv[0].count);
    if (rc == 0) {
        bitcpy(self->val.base, 0, cp->argv[0].data.u8, 0, self->csize);

        rslt->self = self;
        rslt->whack = self_free;
        rslt->variant = vftRow;
        rslt->u.rf = newecho_func;
        return 0;
    }
    free(self);
    return rc;
}

VTRANSFACT_IMPL ( user_newecho, 1, 0, 0 ) ( const void *self, const VXfactInfo *info,
    VFuncDesc *rslt, const VFactoryParams *cp, const VFunctionParams *dp )
{
    return newecho_row_0 ( info, rslt, cp );
}

FIXTURE_TEST_CASE ( AddIntrinsic, TestLinkerFicture )
{   // define user's own schema function

    VTRANSFACT_DECL ( user_newecho );

    VLinkerIntFactory fact = {  user_newecho, "newecho" };

    REQUIRE_RC( VDBManagerAddFactories ( m_mgr,  &fact, 1 ) );

    const string schemaText =
"function < type T > T echo #1.0 < T val > ( * any row_len ) = newecho;\n"
"table T #1 \n"
"{\n"
"    column ascii label = < ascii > echo < 'label' > ();\n"
"};\n"
"database db #1\n"
"{\n"
"    table T #1 tbl;\n"
"};\n"
;
    MakeEmptyTable ( GetName(), schemaText );
    REQUIRE_EQ( string("@#"), ReadRow() );
}

FIXTURE_TEST_CASE ( OverrideIntrinsic, TestLinkerFicture )
{   // VDB-5737: trying to override an intrinsic schema function. will quietly ignore the new definition.
    VTRANSFACT_DECL ( user_newecho );

    VLinkerIntFactory fact = {  user_newecho, "vdb:echo" }; // ignored since vdb_echo already exists

    REQUIRE_RC( VDBManagerAddFactories ( m_mgr,  &fact, 1 ) );

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

    MakeEmptyTable ( GetName(), schemaText );
    REQUIRE_EQ( string("label"), ReadRow() );   // same as directly calling vdb:echo
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
