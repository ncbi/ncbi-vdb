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
* Create test databases for vdb-dump
*/

#include <fstream>

#include <vdb/manager.h>
#include <vdb/schema.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>

#include <arch-impl.h>

#include <kdb/meta.h>

using namespace std;

#define CHECK_RC(call) { rc_t rc = call; if ( rc != 0 ) return rc; }

rc_t
AddRow ( VCursor* p_curs, int64_t p_rowId, uint32_t p_colIdx, const string& p_value )
{
    CHECK_RC ( VCursorSetRowId ( p_curs, p_rowId ) );
    CHECK_RC ( VCursorOpenRow ( p_curs ) );
    CHECK_RC ( VCursorWrite ( p_curs, p_colIdx, 8, p_value.c_str(), 0, p_value.size() ) );
    CHECK_RC ( VCursorCommitRow ( p_curs ) );
    CHECK_RC ( VCursorCloseRow ( p_curs ) );
    CHECK_RC ( VCursorCommit ( p_curs ) );
    return 0;
}

rc_t
MakeDatabase()
{
    const string ScratchDir         = "./data/";
    const string DefaultSchemaText  =
        "version 2;\n"
        "table table1 #1.0.0 { column ascii col; };\n"

        "database root_database #1\n"
        "{\n"
        " table table1 #1 SEQUENCE;\n"
        " };\n"
    ;
    const string DefaultDatabase    = "root_database";

    VDBManager* mgr;
    CHECK_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
    VSchema* schema;
    CHECK_RC ( VDBManagerMakeSchema ( mgr, & schema ) );
    CHECK_RC ( VSchemaParseText ( schema, NULL, DefaultSchemaText.c_str(), DefaultSchemaText.size() ) );

    VDatabase* db;
    CHECK_RC ( VDBManagerCreateDB ( mgr,
                                    & db,
                                    schema,
                                    DefaultDatabase . c_str(),
                                    kcmInit + kcmMD5,
                                    "%s",
                                    ( ScratchDir + "TestDB" ) . c_str() ) );
    {
        KMetadata *meta;
        CHECK_RC( VDatabaseOpenMetadataUpdate ( db, & meta ) );

        KMDataNode *node;

        CHECK_RC( KMetadataOpenNodeUpdate ( meta, & node, "b16") );
        uint16_t b16 = 0x0102;
        CHECK_RC( KMDataNodeWriteB16 ( node, & b16 ) );
        CHECK_RC( KMDataNodeWriteAttr ( node, "bits", "16" ) );
        CHECK_RC( KMDataNodeRelease( node ) );

        CHECK_RC( KMetadataOpenNodeUpdate ( meta, & node, "b32") );
        uint32_t b32 = 0x01020304;
        CHECK_RC( KMDataNodeWriteB32 ( node, & b32 ) );
        CHECK_RC( KMDataNodeRelease( node ) );

        CHECK_RC( KMetadataOpenNodeUpdate ( meta, & node, "b64") );
        uint64_t b64 = 0x0102030405060708;
        CHECK_RC( KMDataNodeWriteB64 ( node, & b64 ) );
        CHECK_RC( KMDataNodeRelease( node ) );

        CHECK_RC( KMetadataOpenNodeUpdate ( meta, & node, "b128") );
        uint128_t b128;
        uint128_sethi( & b128, 0x0102030405060708 );
        uint128_setlo( & b128, 0x1112131415161718 );
        CHECK_RC( KMDataNodeWriteB128 ( node, & b128 ) );
        CHECK_RC( KMDataNodeRelease( node ) );

        CHECK_RC( KMetadataRelease( meta ) );
    }

    {   // TABLE1
        VTable *tab;
        CHECK_RC ( VDatabaseCreateTable ( db, & tab, "SEQUENCE", kcmInit + kcmMD5, "SEQUENCE" ) );

        VCursor *curs;
        CHECK_RC ( VTableCreateCursorWrite ( tab, & curs, kcmInsert ) ) ;
        uint32_t idx;
        CHECK_RC ( VCursorAddColumn ( curs, & idx, "col" ) );
        CHECK_RC ( VCursorOpen ( curs ) );
        CHECK_RC ( AddRow ( curs, 1, idx, "1" ) );
        CHECK_RC ( AddRow ( curs, 2, idx, "2" ) );
        CHECK_RC ( VCursorRelease ( curs ) );

        CHECK_RC ( VTableRelease ( tab ) );
    }

    CHECK_RC ( VSchemaRelease ( schema ) );
    CHECK_RC ( VDatabaseRelease ( db ) );
    CHECK_RC ( VDBManagerRelease ( mgr ) );
    return 0;
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

const char UsageDefaultName[] = "makedb";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();

    return MakeDatabase();
}

}
