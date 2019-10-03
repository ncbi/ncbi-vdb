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

#include <cmath>

#include <vdb/manager.h> // VDBManager
#include <vdb/database.h> 
#include <vdb/table.h> 
#include <vdb/cursor.h> 
#include <vdb/schema.h> /* VSchemaRelease */
#include <vdb/vdb-priv.h>

extern "C" {
    #include <../libs/vdb/table-priv.h>
}

#include <kdb/meta.h>
#include <kdb/table.h>

#include <klib/rc.h>

#include <ktst/unit_test.hpp> // TEST_CASE

#include <sysalloc.h>

#include <sstream>
#include <cstdlib>

using namespace std;

TEST_SUITE( BlobValidationTestSuite )

class VDB_Fixture
{
public:
    VDB_Fixture()
    : m_rdb(NULL)
    {
        THROW_ON_RC ( KDirectoryNativeDir ( & m_wd ) );
        THROW_ON_RC ( VDBManagerMakeRead ( & m_mgr, m_wd ) );
    }
    ~VDB_Fixture()
    {
        VDBManagerRelease ( m_mgr );
        KDirectoryRelease ( m_wd );
    }

    const VCursor* OpenSequenceTblCursor ()
    {
        assert ( m_rdb == NULL );

        // MakeDatabase
        {
            const VDBManager* mgr;
            THROW_ON_RC ( VDBManagerMakeRead ( & mgr, NULL ) );

            THROW_ON_RC ( VDBManagerOpenDBRead ( mgr,
                                              & m_rdb,
                                              NULL,
                                              "%s",
                                              m_databaseName . c_str () ) );
            THROW_ON_RC ( VDBManagerRelease ( mgr ) );
        }

        // MakeCursor
        const VCursor* cursor;
        {
            THROW_ON_RC ( VDatabaseOpenTableRead( m_rdb, & m_tbl, "%s", "SEQUENCE" ) );
            THROW_ON_RC ( VTableCreateCursorRead ( m_tbl, & cursor ) );

        }

        return cursor;
    }

    void Release ( const VCursor* cursor )
    {
        THROW_ON_RC ( VCursorRelease ( cursor ) );
        THROW_ON_RC ( VTableRelease ( m_tbl ) );
        THROW_ON_RC ( VDatabaseRelease ( m_rdb ) );
        m_rdb = NULL;
    }


    KDirectory* m_wd;

    const VDBManager* m_mgr;
    string m_databaseName;

    const VDatabase* m_rdb;
    const VTable* m_tbl;
};

FIXTURE_TEST_CASE ( BlobValidationDisabled, VDB_Fixture)
{
    m_databaseName = "./db/blob_val_inv_chsum.sra";

    uint32_t columnIdx;
    uint32_t elem_bits;
    const uint32_t *col_data;
    uint32_t boff;
    uint32_t row_len;
    const VCursor * cursor;
    // open cursor, disable validation, make sure we can read data
    cursor = OpenSequenceTblCursor();
    REQUIRE_RC ( VCursorAddColumn ( cursor, & columnIdx, "READ_LEN" ) );
    REQUIRE_RC ( VCursorOpen ( cursor ) );
    ((VTable*)m_tbl)->blob_validation = false;
    REQUIRE_RC ( VCursorCellDataDirect ( cursor, 1, columnIdx, &elem_bits, (const void**)&col_data, &boff, &row_len ) );
    REQUIRE_EQ ( (int)row_len, 2 );
    REQUIRE_EQ ( (int)col_data[0], 60 );
    REQUIRE_EQ ( (int)col_data[1], 50 );
    Release ( cursor );
}

FIXTURE_TEST_CASE ( BlobValidationEnabled, VDB_Fixture)
{
    m_databaseName = "./db/blob_val_inv_chsum.sra";

    uint32_t columnIdx;
    uint32_t elem_bits;
    const uint32_t *col_data;
    uint32_t boff;
    uint32_t row_len;
    const VCursor * cursor;
    // open cursor, enable validation, make sure error is thrown, because blob checksum is set to 0
    cursor = OpenSequenceTblCursor();
    REQUIRE_RC ( VCursorAddColumn ( cursor, & columnIdx, "READ_LEN" ) );
    REQUIRE_RC ( VCursorOpen ( cursor ) );
    ((VTable*)m_tbl)->blob_validation = true;
    REQUIRE_RC_FAIL ( VCursorCellDataDirect ( cursor, 1, columnIdx, &elem_bits, (const void**)&col_data, &boff, &row_len ) );
    Release ( cursor );
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

const char UsageDefaultName[] = "test-blob-val";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=BlobValidationTestSuite(argc, argv);
    return rc;
}

}
