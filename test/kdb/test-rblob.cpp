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
* Unit tests for read-side KColumnBlob
*/

#include <ktst/unit_test.hpp>

extern "C"
{
    #include "../libs/kdb/rcolumnblob.h"
    #include "../libs/kdb/rcolumn.h"
}

#include <../libs/kdb/ColumnBlob.hpp>

using namespace std;

TEST_SUITE(RBlobTestSuite);

class R_ColumnBlob: public ColumnBlob<KRColumnBlob>
{
public:
    R_ColumnBlob()  {}
    virtual ~R_ColumnBlob() { release(); }

    rc_t Init(bool bswap)
    {
        return KRColumnBlobMake ( & m_blob, bswap );
    }
};

TEST_CASE( R_ColumnBlob_AddRelease )
{
    R_ColumnBlob b;
    REQUIRE_RC( b.Init(false) );

    REQUIRE_EQ( 1, (int)atomic32_read( & b.refcount ) );
    REQUIRE_RC( b.addRef() );
    REQUIRE_EQ( 2, (int)atomic32_read( & b.refcount ) );
    REQUIRE_RC( b.release() );
    REQUIRE_EQ( 1, (int)atomic32_read( & b.refcount ) );
}

TEST_CASE( R_ColumnBlob_Read )
{
    R_ColumnBlob b;
    REQUIRE_RC( b.Init(false) );

    char buffer[1024];
    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    REQUIRE_EQ( rc, b.read( 0, buffer, sizeof( buffer ), nullptr, nullptr ) );
}

TEST_CASE( R_ColumnBlob_ReadAll )
{
    R_ColumnBlob b;
    REQUIRE_RC( b.Init(false) );

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    REQUIRE_EQ( rc, b.readAll ( nullptr, nullptr, 0 ) );
}

TEST_CASE( R_ColumnBlob_Validate )
{
    R_ColumnBlob b;
    REQUIRE_RC( b.Init(false) );

    REQUIRE_RC( b.validate() );
}

TEST_CASE( R_ColumnBlob_ValidateBuffer)
{
    R_ColumnBlob b;
    REQUIRE_RC( b.Init(false) );

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    REQUIRE_EQ( rc, b.validateBuffer ( nullptr, nullptr, 0 ) );
}

TEST_CASE( R_ColumnBlob_IdRange )
{
    R_ColumnBlob b;
    REQUIRE_RC( b.Init(false) );

    rc_t rc = SILENT_RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    REQUIRE_EQ( rc, b.idRange ( nullptr, nullptr ) );
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

const char UsageDefaultName[] = "Test_KDB_RBlob";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=RBlobTestSuite(argc, argv);
    return rc;
}

}
