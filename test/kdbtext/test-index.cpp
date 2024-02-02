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
* Unit tests for KDBIndex for reading textual data
*/

#include <ktst/unit_test.hpp>

#include "../../libs/kdbtext/index.hpp"

#include <kdb/kdb-priv.h>

#include <klib/rc.h>
#include <klib/json.h>

using namespace std;
using namespace KDBText;

TEST_SUITE(KDBTextIndexTestSuite);

const char * TextIndex =
R"({"name":"idx",
    "text":[
        {"key":"CG", "startId":1, "count":10},
        {"key":"AT", "startId":11, "count":2}
    ]
})";

class KDBTextIndex_Fixture
{
public:
    KDBTextIndex_Fixture()
    {
    }
    ~KDBTextIndex_Fixture()
    {
        delete m_idx;
        KJsonValueWhack( m_json );
    }

    void Setup( const char * input )
    {
        THROW_ON_RC( KJsonValueMake ( & m_json, input, m_error, sizeof m_error ) );
        THROW_ON_FALSE( jsObject == KJsonGetValueType ( m_json ) );

        const KJsonObject * json = KJsonValueToObject ( m_json );
        THROW_ON_FALSE( json != nullptr );

        m_idx = new Index( json );
    }

    KJsonValue * m_json = nullptr;
    Index * m_idx = nullptr;
    char m_error[1024] = {0};
};

FIXTURE_TEST_CASE(KDBTextIndex_Make_Empty, KDBTextIndex_Fixture)
{
    Setup(R"({})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KDBTextIndex_Make_NameBad, KDBTextIndex_Fixture)
{
    Setup(R"({"name":null})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KDBTextIndex_Make, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"idx"})");
    REQUIRE_RC( m_idx -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( string("idx"), m_idx->getName() );
}

// Text index (string -> { id, count })

FIXTURE_TEST_CASE(KDBTextIndex_Make_DataBad, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","text":1})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Make_DataElementNotObject, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","text":[1]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Make_DataElementNoKey, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","text":[{"startId":1, "count":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Make_DataElementKeyBad, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","text":[{"key":null,"startId":1, "count":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Make_DataElementNoStart, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","text":[{"key":"A","count":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Make_DataElementStartBad, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","text":[{"key":"A","startId":"qq","count":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Make_DataElementNoCount, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","text":[{"key":"A","startId":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Make_DataElementCountBad, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","text":[{"key":"A","startId":10,"count":"qq"}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Make_DataElementKeyDuplicate, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","text":[{"key":"a","startId":1, "count":10}, {"key":"a","startId":1, "count":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

// Integer index (offset -> { id, count })

FIXTURE_TEST_CASE(KDBTextIndex_Int_Make_DataBad, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","int":1})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Int_Make_DataElementNotObject, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","int":[1]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Int_Make_DataElementNoKey, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","int":[{"startId":1, "count":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Int_Make_DataElementKeyBad, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","int":[{"key":"qq","startId":1, "count":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Int_Make_DataElementNoKeySize, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","int":[{"key":1,"startId":1, "count":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Int_Make_DataElementKeySizeBad, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","int":[{"key":1,"size":"qq","startId":1, "count":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Int_Make_DataElementNoStart, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","int":[{"key":1,"count":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Int_Make_DataElementStartBad, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","int":[{"key":1,"startId":"qq","count":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Int_Make_DataElementNoCount, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","int":[{"key":1,"size":2,"startId":10}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    // cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Int_Make_DataElementCountBad, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","int":[{"key":1,"size":2,"startId":10,"count":"qq"}]})");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    // cout << m_error << endl;
}
FIXTURE_TEST_CASE(KDBTextIndex_Int_Make_DataElementKeyDuplicate, KDBTextIndex_Fixture)
{
    Setup(R"({"name":"n","int":[{"key":1,"size":2,"startId":1, "count":10}, {"key":1,"size":2,"startId":1, "count":10}]
    })");
    REQUIRE_RC_FAIL( m_idx -> inflate( m_error, sizeof m_error ) );
    //cout << m_error << endl;
}

FIXTURE_TEST_CASE(KDBTextIndex_TextFind, KDBTextIndex_Fixture)
{
    Setup(TextIndex);
    REQUIRE_RC( m_idx -> inflate( m_error, sizeof m_error ) );
    REQUIRE_EQ( (int64_t)1, m_idx->getTextData().find("CG")->second.first );
    REQUIRE_EQ( (uint64_t)10, m_idx->getTextData().find("CG")->second.second );
    REQUIRE_EQ( (int64_t)11, m_idx->getTextData().find("AT")->second.first );
    REQUIRE_EQ( (uint64_t)2, m_idx->getTextData().find("AT")->second.second );
}

// API

class KTextIndex_ApiFixture
{
public:
    KTextIndex_ApiFixture()
    {
    }
    ~KTextIndex_ApiFixture()
    {
        KIndexRelease( m_idx );
        KJsonValueWhack( m_json );
    }
    void Setup( const char * input )
    {
        try
        {
            THROW_ON_RC( KJsonValueMake ( & m_json, input, m_error, sizeof m_error ) );
            THROW_ON_FALSE( jsObject == KJsonGetValueType ( m_json ) );

            const KJsonObject * json = KJsonValueToObject ( m_json );
            THROW_ON_FALSE( json != nullptr );

            Index * idx = new Index( json );
            THROW_ON_RC( idx -> inflate( m_error, sizeof m_error ) );
            m_idx = (const KIndex*)idx;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " with '" << m_error << "'" << endl;
            throw;
        }

    }

    KJsonValue * m_json = nullptr;
    const KIndex * m_idx = nullptr;
    char m_error[1024] = {0};

    int64_t m_start_id = 0;
    uint64_t m_id_count = 0;
};

FIXTURE_TEST_CASE(KIndex_AddRelease, KTextIndex_ApiFixture)
{
    Setup(R"({"name":"idx"})");

    REQUIRE_RC( KIndexAddRef( m_idx ) );
    REQUIRE_RC( KIndexRelease( m_idx ) );
    // use valgrind to find any leaks
}

FIXTURE_TEST_CASE(KIndex_Locked, KTextIndex_ApiFixture)
{   // always false for this library
    Setup(R"({"name":"idx"})");
    REQUIRE( ! KIndexLocked( m_idx ) );
}

FIXTURE_TEST_CASE(KIndex_Version_Null, KTextIndex_ApiFixture)
{   // always 0 for this library
    Setup(R"({"name":"idx"})");
    REQUIRE_RC_FAIL( KIndexVersion( m_idx, nullptr ) );
}

FIXTURE_TEST_CASE(KIndex_Version, KTextIndex_ApiFixture)
{   // always 0 for this library
    Setup(R"({"name":"idx"})");
    uint32_t version = 1;
    REQUIRE_RC( KIndexVersion( m_idx, & version ) );
    REQUIRE_EQ( (uint32_t)0, version );
}

FIXTURE_TEST_CASE(KIndex_Type_Null, KTextIndex_ApiFixture)
{
    Setup(R"({"name":"idx"})");
    REQUIRE_RC_FAIL( KIndexType( m_idx, nullptr ) );
}

FIXTURE_TEST_CASE(KIndex_Type, KTextIndex_ApiFixture)
{   // always kitText for now
    Setup(R"({"name":"idx"})");
    KIdxType type = 99;
    REQUIRE_RC( KIndexType( m_idx, & type ) );
    REQUIRE_EQ( (int)kitText, (int)type );
}

FIXTURE_TEST_CASE(KIndex_ConsistencyCheck, KTextIndex_ApiFixture)
{   // no op, for now
    Setup(R"({"name":"idx"})");
    uint64_t num_keys = 0;
    uint64_t num_rows = 0;
    uint64_t num_holes = 0;
    REQUIRE_RC( KIndexConsistencyCheck( m_idx, 1, &m_start_id, &m_id_count, &num_keys, &num_rows, &num_holes ) );
}

// KIndexFindText

FIXTURE_TEST_CASE(KIndex_FindText_StartNull, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    REQUIRE_RC_FAIL( KIndexFindText ( m_idx, "CG", nullptr, &m_id_count, nullptr, nullptr ) );
}
FIXTURE_TEST_CASE(KIndex_FindText_KeyNull, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    REQUIRE_RC_FAIL( KIndexFindText ( m_idx, nullptr, &m_start_id, &m_id_count, nullptr, nullptr ) );
}
FIXTURE_TEST_CASE(KIndex_FindText_KeyEmpty, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    REQUIRE_RC_FAIL( KIndexFindText ( m_idx, "", &m_start_id, &m_id_count, nullptr, nullptr ) );
}
FIXTURE_TEST_CASE(KIndex_FindText_NotFound, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    REQUIRE_RC_FAIL( KIndexFindText ( m_idx, "TT", &m_start_id, &m_id_count, nullptr, nullptr ) );
}
FIXTURE_TEST_CASE(KIndex_FindText_Found, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    REQUIRE_RC( KIndexFindText ( m_idx, "AT", &m_start_id, &m_id_count, nullptr, nullptr ) );
    REQUIRE_EQ( (int64_t)11, m_start_id );
    REQUIRE_EQ( (uint64_t)2, m_id_count );
}

int CC Compare ( const void *item, struct PBSTNode const *n, void *data )
{
    return 0;
}
FIXTURE_TEST_CASE(KIndex_FindText_CustomCompare, KTextIndex_ApiFixture)
{   // not supported
    Setup(TextIndex);
    REQUIRE_RC_FAIL( KIndexFindText( m_idx, "AT", &m_start_id, &m_id_count, Compare, nullptr ) );
}

// KIndexFindAllText

int64_t cb_id = 0;
uint64_t cb_id_count = 0;
void * cb_data = nullptr;
rc_t cb_rc = 0;
rc_t CC FindAllTextCallback ( int64_t id, uint64_t id_count, void *data )
{
    cb_id = id;
    cb_id_count = id_count;
    cb_data = data;
    return cb_rc;
}
FIXTURE_TEST_CASE(KIndex_FindAllText_KeyNull, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    REQUIRE_RC_FAIL( KIndexFindAllText ( m_idx, nullptr, FindAllTextCallback, nullptr ) );
}
FIXTURE_TEST_CASE(KIndex_FindAllText_KeyEmpty, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    REQUIRE_RC_FAIL( KIndexFindAllText ( m_idx, "", FindAllTextCallback, nullptr ) );
}
FIXTURE_TEST_CASE(KIndex_FindAllText_NoCallback, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    REQUIRE_RC_FAIL( KIndexFindAllText ( m_idx, "qq", nullptr, nullptr ) );
}

FIXTURE_TEST_CASE(KIndex_FindAllText, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    char TextData = 0;
    REQUIRE_RC( KIndexFindAllText ( m_idx, "AT", FindAllTextCallback, &TextData ) );
    REQUIRE_EQ( (int64_t)11, cb_id );
    REQUIRE_EQ( (uint64_t)2, cb_id_count );
    REQUIRE_EQ( (void*)&TextData, cb_data );
}
FIXTURE_TEST_CASE(KIndex_FindAllText_NotFound, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    char TextData = 0;
    REQUIRE_RC_FAIL( KIndexFindAllText ( m_idx, "ATT", FindAllTextCallback, &TextData ) );
}
FIXTURE_TEST_CASE(KIndex_FindAllText_CallbackFails, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    char TextData = 0;
    cb_rc = SILENT_RC ( rcDB, rcIndex, rcSelecting, rcBlob, rcAmbiguous );
    REQUIRE_EQ( cb_rc, KIndexFindAllText ( m_idx, "AT", FindAllTextCallback, &TextData ) );
    cb_rc = 0; // restore
}

// KTextIndexProjectText

FIXTURE_TEST_CASE(KIndex_ProjectText_KeyNull, KTextIndex_ApiFixture)
{
    Setup(TextIndex);

    int64_t start_id;
    uint64_t id_count;
    size_t actsize;
    REQUIRE_RC_FAIL( KIndexProjectText ( m_idx, 1, &start_id, &id_count, nullptr, 0, &actsize ) );
}

FIXTURE_TEST_CASE(KIndex_ProjectText_Found, KTextIndex_ApiFixture)
{
    Setup(TextIndex);

    int64_t start_id;
    uint64_t id_count;
    char key[1024];
    size_t actsize;
    REQUIRE_RC( KIndexProjectText ( m_idx, 7, &start_id, &id_count, key, sizeof key, &actsize ) );

    REQUIRE_EQ( (int64_t)1, start_id );
    REQUIRE_EQ( (uint64_t)10, id_count );
    REQUIRE_EQ( string("CG"), string(key, actsize) );
}

FIXTURE_TEST_CASE(KIndex_ProjectText_NotFound, KTextIndex_ApiFixture)
{
    Setup(TextIndex);

    int64_t start_id;
    uint64_t id_count;
    char key[1024];
    size_t actsize;
    REQUIRE_RC_FAIL( KIndexProjectText ( m_idx, 77, &start_id, &id_count, key, sizeof key, &actsize ) );
}

FIXTURE_TEST_CASE(KIndex_ProjectText_NoOutputs, KTextIndex_ApiFixture)
{
    Setup(TextIndex);

    char key[1024];
    REQUIRE_RC( KIndexProjectText ( m_idx, 7, nullptr, nullptr, key, sizeof key, nullptr ) );
    REQUIRE_EQ( string("CG"), string(key) );
}

FIXTURE_TEST_CASE(KIndex_ProjectText_BufferShort, KTextIndex_ApiFixture)
{
    Setup(TextIndex);

    char key[1];
    size_t actsize;
    REQUIRE_RC_FAIL( KIndexProjectText ( m_idx, 7, nullptr, nullptr, key, sizeof key, &actsize ) );
    REQUIRE_EQ( (size_t)2, actsize );
}

// KTextIndexProjectAllText

FIXTURE_TEST_CASE(KIndex_ProjectAllText_CallbackNull, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    REQUIRE_RC_FAIL( KIndexProjectAllText ( m_idx, 7, nullptr, nullptr ) );
}

int64_t pcb_id = 0;
uint64_t pcb_id_count = 0;
string pcb_key;
void * pcb_data = nullptr;
rc_t pcb_rc = 0;
rc_t CC ProjectAllTextCallback ( int64_t start_id, uint64_t id_count, const char *key, void *data )
{
    pcb_id = start_id;
    pcb_id_count = id_count;
    pcb_key = key;
    pcb_data = data;
    return pcb_rc;
}
FIXTURE_TEST_CASE(KIndex_ProjectAllText_Found, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    char data;
    REQUIRE_RC( KIndexProjectAllText ( m_idx, 7, ProjectAllTextCallback, & data ) );

    REQUIRE_EQ( (int64_t)1, pcb_id );
    REQUIRE_EQ( (uint64_t)10, pcb_id_count );
    REQUIRE_EQ( string("CG"), pcb_key );
    REQUIRE_EQ( (void*)&data, pcb_data );
}
FIXTURE_TEST_CASE(KIndex_ProjectAllText_CallbackRc, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    char data;
    pcb_rc = 1;
    REQUIRE_RC_FAIL( KIndexProjectAllText ( m_idx, 7, ProjectAllTextCallback, & data ) );
    pcb_rc = 0;
}
FIXTURE_TEST_CASE(KIndex_ProjectAllText_NotFound, KTextIndex_ApiFixture)
{
    Setup(TextIndex);
    char data;
    REQUIRE_RC_FAIL( KIndexProjectAllText ( m_idx, 77, ProjectAllTextCallback, & data ) );
}

// KTextIndexFindU64

const char * NumIndex =
R"({"name":"idx",
    "int":[
        {"key":1, "size":5, "startId":1, "count":2},
        {"key":1, "size":10,"startId":2, "count":3},
        {"key":6, "size":3, "startId":3, "count":4},
        {"key":12,"size":4, "startId":4, "count":5}
    ]
})";

FIXTURE_TEST_CASE(KIndex_FindU64, KTextIndex_ApiFixture)
{ // find first
    Setup(NumIndex);
    uint64_t key;
    uint64_t key_size;
    int64_t id;
    uint64_t id_qty;
    REQUIRE_RC( KIndexFindU64 ( m_idx, 7, &key, &key_size, &id, &id_qty ) );
    REQUIRE_EQ( (uint64_t)1, key );
    REQUIRE_EQ( (uint64_t)10, key_size );
    REQUIRE_EQ( (int64_t)2, id );
    REQUIRE_EQ( (uint64_t)3, id_qty );
}
FIXTURE_TEST_CASE(KIndex_FindU64_NotFound, KTextIndex_ApiFixture)
{ // find first
    Setup(NumIndex);
    uint64_t key;
    uint64_t key_size;
    int64_t id;
    uint64_t id_qty;
    REQUIRE_RC_FAIL( KIndexFindU64 ( m_idx, 11, &key, &key_size, &id, &id_qty ) ); // in a gap
    REQUIRE_RC_FAIL( KIndexFindU64 ( m_idx, 16, &key, &key_size, &id, &id_qty ) ); // beyond last
}

// KTextIndexFindAllU64

FIXTURE_TEST_CASE(KIndex_ProjectAllU64_CallbackNull, KTextIndex_ApiFixture)
{
    Setup(NumIndex);
    REQUIRE_RC_FAIL( KIndexFindAllU64 ( m_idx, 1, nullptr, nullptr ) );
}

struct U64Data
{
    uint64_t key;
    uint64_t key_size;
    int64_t id;
    uint64_t id_qty;
    void* data;
};
vector< U64Data > u64CallbackData;
rc_t u64cb_rc = 0;

static
rc_t CC
FindALlU64Callback(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data )
{
    U64Data d;
    d.key = key;
    d.key_size = key_size;
    d.id = id;
    d.id_qty = id_qty;
    d.data = data;
    u64CallbackData.push_back(d);
    return u64cb_rc;
}
FIXTURE_TEST_CASE(KIndex_ProjectAllU64, KTextIndex_ApiFixture)
{
    Setup(NumIndex);
    char Data = 0;
    u64CallbackData.clear();
    REQUIRE_RC( KIndexFindAllU64 ( m_idx, 7, FindALlU64Callback, &Data ) );
    REQUIRE_EQ( (size_t)2, u64CallbackData.size() );
    REQUIRE_EQ( (uint64_t)1, u64CallbackData[0].key );
    REQUIRE_EQ( (uint64_t)10, u64CallbackData[0].key_size );
    REQUIRE_EQ( (int64_t)2, u64CallbackData[0].id );
    REQUIRE_EQ( (uint64_t)3, u64CallbackData[0].id_qty );
    REQUIRE_EQ( (void*)&Data, u64CallbackData[0].data );
    REQUIRE_EQ( (uint64_t)6, u64CallbackData[1].key );
    REQUIRE_EQ( (uint64_t)3, u64CallbackData[1].key_size );
    REQUIRE_EQ( (int64_t)3, u64CallbackData[1].id );
    REQUIRE_EQ( (uint64_t)4, u64CallbackData[1].id_qty );
    REQUIRE_EQ( (void*)&Data, u64CallbackData[1].data );
}
FIXTURE_TEST_CASE(KIndex_ProjectAllU64_CallbackFails, KTextIndex_ApiFixture)
{
    Setup(NumIndex);
    char Data = 0;
    u64CallbackData.clear();
    u64cb_rc = 1;
    REQUIRE_EQ( (rc_t)1, KIndexFindAllU64 ( m_idx, 7, FindALlU64Callback, &Data ) );
    REQUIRE_EQ( (size_t)1, u64CallbackData.size() ); //called once
}

FIXTURE_TEST_CASE(KIndex_SetMaxRowId, KTextIndex_ApiFixture)
{   // no op
    Setup(TextIndex);
    KIndexSetMaxRowId ( m_idx, 100 );
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

const char UsageDefaultName[] = "Test_KDBText_Index";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KDBTextIndexTestSuite(argc, argv);
    return rc;
}

}
