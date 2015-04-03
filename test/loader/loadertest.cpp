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
* Unit tests for the Loader module
*/
#include <ktst/unit_test.hpp>

#include <vdb/manager.h> // VDBManager
#include <vdb/database.h> 
#include <vdb/schema.h> /* VSchemaRelease */

extern "C" {
#include <loader/sequence-writer.h>
}

using namespace std;

TEST_SUITE(LoaderTestSuite);

TEST_CASE ( SequenceWriter_Write ) 
{
    const char * schemaFile = "sequencewriter.vschema";
    const char * schemaSpec = "NCBI:align:db:alignment_sorted";
    const char * databaseName = GetName();
    VDatabase* db;
    {
        VDBManager* mgr;
        REQUIRE_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
        VSchema* schema;
        REQUIRE_RC ( VDBManagerMakeSchema ( mgr, & schema ) );
        REQUIRE_RC ( VSchemaParseFile(schema, "%s", schemaFile ) );
        
        REQUIRE_RC ( VDBManagerCreateDB ( mgr, 
                                          & db, 
                                          schema, 
                                          schemaSpec, 
                                          kcmInit + kcmMD5, 
                                          "%s", 
                                          databaseName ) );
        REQUIRE_RC ( VSchemaRelease ( schema ) );
        REQUIRE_RC ( VDBManagerRelease ( mgr ) );
    }
    
    
    SequenceWriter wr;
    
    REQUIRE_NOT_NULL ( SequenceWriterInit ( & wr, db ) );

    uint8_t qual[2] = {0,0};
    uint32_t readStart = 0;
    uint32_t readLen = 2;
    uint8_t orientation = 0;
    uint8_t is_bad = 0;
    uint8_t alignmentCount = 0;
    bool aligned = false;
    uint64_t ti = 0;
    
    SequenceRecord rec;
    rec . seq = (char*)"AC";
    rec . qual = qual;
    rec . readStart = & readStart;
    rec . readLen = & readLen;
    rec . orientation = & orientation;
    rec . is_bad = & is_bad;
    rec . alignmentCount = & alignmentCount;
    rec . spotGroup = (char*)"SG";
    rec . aligned = & aligned;
    rec . cskey = (char*)"";
    rec . ti = & ti;
    rec . spotName = (char*)"name";
    rec . keyId = 1;
    rec . spotGroupLen = 2;
    rec . spotNameLen = 4;
    rec . numreads = 1;

    REQUIRE_RC ( KDataBufferMake ( & rec.storage, 8, 0 ) );
    
    REQUIRE_RC ( SequenceWriteRecord ( & wr, 
                                       & rec,
                                       false, 
                                       false, 
                                       SRA_PLATFORM_454,
                                       false,
                                       false,
                                       false,
                                       "0"
                                      ) );
    REQUIRE_RC ( SequenceDoneWriting ( & wr ) );
    SequenceWhack ( & wr, true );
    
    //TODO: read, validate
    
    REQUIRE_RC ( VDatabaseRelease ( db ) );
    
    {
        KDirectory* wd;
        REQUIRE_RC ( KDirectoryNativeDir ( & wd ) );
        REQUIRE_RC ( KDirectoryRemove ( wd, true, GetName() ) );
    }
}

//////////////////////////////////////////// Main
#include <kapp/args.h>
#include <klib/out.h>

extern "C"
{

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}

const char UsageDefaultName[] = "test-loader";

rc_t CC UsageSummary (const char * progname)
{
    return KOutMsg ( "Usage:\n" "\t%s [options]\n\n", progname );
}

rc_t CC Usage( const Args* args )
{
    return 0;
}

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc=LoaderTestSuite(argc, argv);
    return rc;
}

}

