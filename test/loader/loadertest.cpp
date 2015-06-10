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

#include <stdexcept> 

extern "C" {
#include <loader/sequence-writer.h>
}

using namespace std;

TEST_SUITE(LoaderTestSuite);

class LoaderFixture
{
public:
    LoaderFixture()
    :   m_db ( 0 ),
        m_cursor ( 0 ),
        m_keepDatabase ( false )
    {
    }
    ~LoaderFixture()
    {
        RemoveDatabase();
    }
    
    void RemoveDatabase()
    {
        CloseDatabase();
        if ( ! m_databaseName . empty () && ! m_keepDatabase )
        {
            KDirectory* wd;
            KDirectoryNativeDir ( & wd );
            KDirectoryRemove ( wd, true, m_databaseName . c_str() );
            KDirectoryRelease ( wd );
        }
    }
    
    void OpenDatabase()
    {
        CloseDatabase();
        
        VDBManager * vdb;
        if ( VDBManagerMakeUpdate ( & vdb, NULL ) != 0 )
            throw logic_error("LoaderFixture::OpenDatabase(" + m_databaseName + "): VDBManagerMakeUpdate failed");
           
        if ( VDBManagerOpenDBUpdate ( vdb, &m_db, NULL, m_databaseName . c_str() ) != 0 )
            throw logic_error("LoaderFixture::OpenDatabase(" + m_databaseName + "): VDBManagerOpenDBUpdate failed");
        
        if ( VDBManagerRelease ( vdb ) != 0 )
            throw logic_error("LoaderFixture::OpenDatabase(" + m_databaseName + "): VDBManagerRelease failed");
    }
    void CloseDatabase()
    {
        if ( m_db != 0 )
        {
            VDatabaseRelease ( m_db );
            m_db = 0;
        }
        if ( m_cursor != 0 )
        {
            VCursorRelease ( m_cursor );
            m_cursor = 0;
        }
    }
    
    void OpenCursor( const char* p_table, const char* p_column )
    {
        OpenDatabase();
        const VTable * tbl;
        if ( VDatabaseOpenTableRead ( m_db, &tbl, p_table ) != 0 )
            throw logic_error(string ( "LoaderFixture::OpenCursor(" ) + p_table + "): VDatabaseOpenTableRead failed");
        if ( VTableCreateCursorRead ( tbl, & m_cursor ) != 0 )
            throw logic_error(string ( "LoaderFixture::OpenCursor(" ) + p_table + "): VTableCreateCursorRead failed");
        
        uint32_t idx;
        if ( VCursorAddColumn ( m_cursor, &idx, p_column ) != 0 ) 
            throw logic_error(string ( "LoaderFixture::OpenCursor(" ) + p_column + "): VCursorAddColumn failed");
        
        if ( VCursorOpen ( m_cursor ) != 0 )
            throw logic_error(string ( "LoaderFixture::OpenCursor(" ) + p_table + "): VCursorOpen failed");
        
        if ( VTableRelease ( tbl ) != 0 )
            throw logic_error(string ( "LoaderFixture::OpenCursor(" ) + p_table + "): VTableRelease failed");
    }
    
    template < typename T > T GetValue ( const char* p_table, const char* p_column, uint64_t p_row )
    {
        OpenCursor( p_table, p_column ); 
        if ( VCursorSetRowId ( m_cursor, p_row ) ) 
            throw logic_error("LoaderFixture::GetValueU32(): VCursorSetRowId failed");
        
        if ( VCursorOpenRow ( m_cursor ) != 0 )
            throw logic_error("LoaderFixture::GetValueU32(): VCursorOpenRow failed");
            
        T ret;
        uint32_t num_read;
        if ( VCursorRead ( m_cursor, 1, 8 * sizeof ret, &ret, 1, &num_read ) != 0 )
            throw logic_error("LoaderFixture::GetValueU32(): VCursorRead failed");
        
        if ( VCursorCloseRow ( m_cursor ) != 0 )
            throw logic_error("LoaderFixture::GetValueU32(): VCursorCloseRow failed");

         return ret;
    }
    
    template < typename T, int Count > bool GetValue ( const char* p_table, const char* p_column, uint64_t p_row, T ret[Count] )
    {
        OpenCursor( p_table, p_column ); 
        if ( VCursorSetRowId ( m_cursor, p_row ) ) 
            throw logic_error("LoaderFixture::GetValueU32(): VCursorSetRowId failed");
        
        if ( VCursorOpenRow ( m_cursor ) != 0 )
            throw logic_error("LoaderFixture::GetValueU32(): VCursorOpenRow failed");
            
        uint32_t num_read;
        if ( VCursorRead ( m_cursor, 1, 8 * sizeof ( T ), ret, Count, &num_read ) != 0 || 
             num_read != Count)
            throw logic_error("LoaderFixture::GetValueU32(): VCursorRead failed");
        
        if ( VCursorCloseRow ( m_cursor ) != 0 )
            throw logic_error("LoaderFixture::GetValueU32(): VCursorCloseRow failed");

         return ret;
    }
    
    string          m_databaseName;
    VDatabase *     m_db;
    const VCursor * m_cursor;
    bool            m_keepDatabase;
};    

template<> std::string LoaderFixture::GetValue ( const char* p_table, const char* p_column, uint64_t p_row )
{
    OpenCursor( p_table, p_column ); 
    if ( VCursorSetRowId ( m_cursor, p_row ) ) 
        throw logic_error("LoaderFixture::GetValue(): VCursorSetRowId failed");
    
    if ( VCursorOpenRow ( m_cursor ) != 0 )
        throw logic_error("LoaderFixture::GetValue(): VCursorOpenRow failed");
        
    char buf[1024];
    uint32_t num_read;
    if ( VCursorRead ( m_cursor, 1, 8, &buf, sizeof buf, &num_read ) != 0 )
        throw logic_error("LoaderFixture::GetValue(): VCursorRead failed");
    
    if ( VCursorCloseRow ( m_cursor ) != 0 )
        throw logic_error("LoaderFixture::GetValue(): VCursorCloseRow failed");

     return string ( buf, num_read );
}


FIXTURE_TEST_CASE ( SequenceWriter_Write, LoaderFixture ) 
{
    m_databaseName = GetName();
    RemoveDatabase();
    const string Sequence = "AC";
    const string SpotName = "name1";
    const string SpotGroup = "spotgroup1";
    const uint8_t qual[2] = {10,20};
//m_keepDatabase = true;

    {
        const char * schemaFile = "./sequencewriter.vschema";
        const char * schemaSpec = "NCBI:align:db:fastq";
        {
            VDBManager* mgr;
            REQUIRE_RC ( VDBManagerMakeUpdate ( & mgr, NULL ) );
            VSchema* schema;
            REQUIRE_RC ( VDBManagerMakeSchema ( mgr, & schema ) );
            REQUIRE_RC ( VSchemaParseFile(schema, "%s", schemaFile ) );
            
            REQUIRE_RC ( VDBManagerCreateDB ( mgr, 
                                              & m_db, 
                                              schema, 
                                              schemaSpec, 
                                              kcmInit + kcmMD5, 
                                              "%s", 
                                              m_databaseName . c_str() ) );
                                              
            REQUIRE_RC ( VSchemaRelease ( schema ) );
            REQUIRE_RC ( VDBManagerRelease ( mgr ) );
        }
    
        SequenceWriter wr;
        
        REQUIRE_NOT_NULL ( SequenceWriterInit ( & wr, m_db ) );

        uint32_t readStart = 0;
        uint32_t readLen = Sequence . size();
        uint8_t orientation = 0;
        uint8_t is_bad = 0;
        uint8_t alignmentCount = 0;
        bool aligned = false;
        uint64_t ti = 0;
        
        SequenceRecord rec;
        rec . seq = (char*) Sequence . c_str ();
        rec . qual = (uint8_t*) qual;
        rec . readStart = & readStart;
        rec . readLen = & readLen;
        rec . orientation = & orientation;
        rec . is_bad = & is_bad;
        rec . alignmentCount = & alignmentCount;
        rec . spotGroup = (char*) SpotGroup . c_str();
        rec . aligned = & aligned;
        rec . cskey = (char*)"";
        rec . ti = & ti;
        rec . spotName = (char*) SpotName . c_str();
        rec . keyId = 1;
        rec . spotGroupLen = SpotGroup . size();;
        rec . spotNameLen = SpotName . size();
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
                                           "0"/*,
                                           false,
                                           true*/
                                          ) );
        REQUIRE_RC ( SequenceDoneWriting ( & wr ) );
        SequenceWhack ( & wr, true );
    
        CloseDatabase();
    }
    
    // read, validate
    REQUIRE_EQ ( Sequence, GetValue<string> ( "SEQUENCE", "CMP_READ", 1 ) );    
    REQUIRE_EQ ( SpotName, GetValue<string> ( "SEQUENCE", "NAME", 1 ) );    
    REQUIRE_EQ ( SpotGroup, GetValue<string> ( "SEQUENCE", "SPOT_GROUP", 1 ) );    
    {
        uint8_t q[2] = { 0, 0 };
        bool req = GetValue < uint8_t, 2 > ( "SEQUENCE", "QUALITY", 1, q );
        REQUIRE ( req );
        REQUIRE_EQ ( (int)qual[0], (int)q[0] );
        REQUIRE_EQ ( (int)qual[1], (int)q[1] );
    }
}

//////////////////////////////////////////// Main
#include <kapp/args.h>
#include <klib/out.h>
#include <kfg/config.h>

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
    KConfigDisableUserSettings();
    rc_t rc=LoaderTestSuite(argc, argv);
    return rc;
}

}

