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
* Unit tests for low-level NGS functions handling blob-bases access to REFERENCE table
*/

// suppress macro max from windows.h
#define NOMINMAX

#include "ngs_c_fixture.hpp"

#include <vdb/database.h>

#include <NGS_Cursor.h>
#include <NGS_ReferenceBlob.h>

#include <../libs/ngs/CSRA1_Reference.h>

#include <stdexcept>

using namespace std;
using namespace ncbi::NK;

TEST_SUITE(NgsReferenceBlobTestSuite);

const char* CSRA1_Accession = "SRR1063272";
const char* CSRA1_Accession_WithRepeats = "SRR600094";

//////////////////////////////////////////// NGS_ReferenceBlob

class ReferenceBlobFixture : public NGS_C_Fixture
{
public:
    ReferenceBlobFixture ()
    :   m_curs ( 0 ),
        m_blob ( 0 )
    {
    }

    virtual void Release()
    {
        if (m_ctx != 0)
        {
            if ( m_blob != 0 )
            {
                NGS_ReferenceBlobRelease ( m_blob, m_ctx );
            }
            if ( m_curs != 0 )
            {
                NGS_CursorRelease ( m_curs, m_ctx );
            }
        }
        NGS_C_Fixture :: Release ();
    }

    void GetCursor(const char* p_acc )
    {
        const VDatabase* db;
        THROW_ON_RC ( VDBManagerOpenDBRead ( m_ctx -> rsrc -> vdb, & db, NULL, p_acc ) );
        NGS_String* run_name = NGS_StringMake ( m_ctx, "", 0);

        if ( m_curs != 0 )
        {
            NGS_CursorRelease ( m_curs, m_ctx );
        }
        m_curs = NGS_CursorMakeDb ( m_ctx, db, run_name, "REFERENCE", reference_col_specs, reference_NUM_COLS );

        NGS_StringRelease ( run_name, m_ctx );
        THROW_ON_RC ( VDatabaseRelease ( db ) );
    }
    void GetBlob ( const char* p_acc, int64_t p_rowId )
    {
        GetCursor ( p_acc );
        if ( m_blob != 0 )
        {
            NGS_ReferenceBlobRelease ( m_blob, m_ctx );
        }
        m_blob = NGS_ReferenceBlobMake ( m_ctx, m_curs, p_rowId );
    }

    const NGS_Cursor* m_curs;
    struct NGS_ReferenceBlob* m_blob;
};

// Make

TEST_CASE ( NGS_ReferenceBlob_Make_BadCursor)
{
    HYBRID_FUNC_ENTRY ( rcSRA, rcRow, rcAccessing );

    struct NGS_ReferenceBlob * blob = NGS_ReferenceBlobMake ( ctx, NULL, 1 );
    REQUIRE_FAILED ();
    REQUIRE_NULL ( blob );
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlob_Make_BadRowId, ReferenceBlobFixture )
{
    ENTRY;
    GetCursor ( CSRA1_Accession );

    struct NGS_ReferenceBlob * blob = NGS_ReferenceBlobMake ( ctx, m_curs, 0 );
    REQUIRE_FAILED ();
    REQUIRE_NULL ( blob );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlob_Make, ReferenceBlobFixture )
{
    ENTRY;
    GetCursor ( CSRA1_Accession );

    m_blob = NGS_ReferenceBlobMake ( ctx, m_curs, 1 );
    REQUIRE_NOT_NULL ( m_blob );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_DuplicateRelease, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    // Duplicate
    NGS_ReferenceBlob* anotherBlob = NGS_ReferenceBlobDuplicate ( m_blob, m_ctx );
    REQUIRE_NOT_NULL ( anotherBlob );
    // Release
    NGS_ReferenceBlobRelease ( anotherBlob, m_ctx );

    EXIT;
}

// Range

FIXTURE_TEST_CASE ( NGS_ReferenceBlob_Range_BadArg_1, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    uint64_t rowCount;
    NGS_ReferenceBlobRowRange ( m_blob, ctx, NULL, &rowCount );
    REQUIRE_EQ ( (uint64_t)4, rowCount);

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlob_Range_BadArg_2, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    int64_t firstRow;
    NGS_ReferenceBlobRowRange ( m_blob, ctx, &firstRow, NULL );
    REQUIRE_EQ ( (int64_t)1, firstRow);

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlob_Range_BadArg_3, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    NGS_ReferenceBlobRowRange ( m_blob, ctx, NULL, NULL );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlob_Range, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    int64_t firstRow;
    uint64_t rowCount;
    NGS_ReferenceBlobRowRange ( m_blob, ctx, &firstRow, &rowCount );
    REQUIRE_EQ ( (int64_t)1, firstRow );
    REQUIRE_EQ ( (uint64_t)4, rowCount );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlob_Range_FirstRowInsideBlob, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 3 );

    int64_t firstRow;
    uint64_t rowCount;
    NGS_ReferenceBlobRowRange ( m_blob, ctx, &firstRow, &rowCount );
    REQUIRE_EQ ( (int64_t)3, firstRow );
    REQUIRE_EQ ( (uint64_t)1, rowCount );

    EXIT;
}

// Data

FIXTURE_TEST_CASE ( NGS_ReferenceBlob_Data_BadArg, ReferenceBlobFixture )
{
    ENTRY;

    const void* data = NGS_ReferenceBlobData ( NULL, ctx );
    REQUIRE_FAILED ();
    REQUIRE_NULL ( data );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlob_Data, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    const void* data = NGS_ReferenceBlobData ( m_blob, ctx );
    REQUIRE_NOT_NULL ( data );
    REQUIRE_EQ ( string ( "GAATTCTAAA" ), string ( (const char*)data, 10 ) );

    EXIT;
}

// Size

FIXTURE_TEST_CASE ( NGS_ReferenceBlob_Size_BadArg, ReferenceBlobFixture )
{
    ENTRY;

    NGS_ReferenceBlobSize ( NULL, m_ctx );
    REQUIRE_FAILED ();

    EXIT;
}
FIXTURE_TEST_CASE ( NGS_ReferenceBlob_Size, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    REQUIRE_EQ ( (uint64_t)20000, NGS_ReferenceBlobSize ( m_blob, m_ctx ) );

    EXIT;
}

// ResolveOffset

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_ResolveOffset_BadSelf, ReferenceBlobFixture )
{
    ENTRY;

    uint64_t inRef;
    NGS_ReferenceBlobResolveOffset ( NULL, ctx, 0, & inRef, NULL, NULL );
    REQUIRE_FAILED ();

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_ResolveOffset_OffsetOutOfRange, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    uint64_t inRef;
    NGS_ReferenceBlobResolveOffset ( m_blob, ctx, 20000, & inRef, NULL, NULL );
    REQUIRE_FAILED ();

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_ResolveOffset_NullOutputParam, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    NGS_ReferenceBlobResolveOffset ( m_blob, ctx, 0, NULL, NULL, NULL );
    REQUIRE_FAILED ();

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_ResolveOffset_NullOptionalParams, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    uint64_t inRef;
    NGS_ReferenceBlobResolveOffset ( m_blob, ctx, 0, & inRef, NULL, NULL );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_ResolveOffset_FirstChunk_NoRepeat, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 ); /* ref positions 1-20000 in this blob */

    uint64_t inRef;
    uint32_t repeatCount;
    uint64_t increment;
    NGS_ReferenceBlobResolveOffset ( m_blob, ctx, 6123, & inRef, & repeatCount, & increment );
    REQUIRE_EQ ( (uint64_t)6123, inRef );
    REQUIRE_EQ ( (uint32_t)1, repeatCount  );
    REQUIRE_EQ ( (uint64_t)0, increment  );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_ResolveOffset_NoRepeat, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 5 ); /* ref positions 20001-25000 in this blob */

    uint64_t inRef;
    uint32_t repeatCount;
    uint64_t increment;
    NGS_ReferenceBlobResolveOffset ( m_blob, ctx, 123, & inRef, & repeatCount, & increment );
    REQUIRE_EQ ( (uint64_t)20123, inRef );
    REQUIRE_EQ ( (uint32_t)1, repeatCount  );
    REQUIRE_EQ ( (uint64_t)0, increment  );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_ResolveOffset_WithRepeat, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession_WithRepeats, 1 );

    uint64_t inRef;
    uint32_t repeatCount;
    uint64_t increment;
    NGS_ReferenceBlobResolveOffset ( m_blob, ctx, 0, & inRef, & repeatCount, & increment );
    REQUIRE_EQ ( (uint64_t)0, inRef );
    REQUIRE_EQ ( (uint32_t)2, repeatCount  );
    REQUIRE_EQ ( (uint64_t)5000, increment  );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_ResolveOffset_PastRepeat, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession_WithRepeats, 1 );

    uint64_t inRef;
    uint32_t repeatCount;
    uint64_t increment;
    NGS_ReferenceBlobResolveOffset ( m_blob, ctx, 5100, & inRef, & repeatCount, & increment );
    REQUIRE_EQ ( (uint64_t)10100, inRef );
    REQUIRE_EQ ( (uint32_t)1, repeatCount  );
    REQUIRE_EQ ( (uint64_t)0, increment  );

    EXIT;
}

// FindRepeat

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_FindRepeat_BadSelf, ReferenceBlobFixture )
{
    ENTRY;

    uint64_t nextInBlob;
    uint64_t inRef;
    REQUIRE ( ! NGS_ReferenceBlobFindRepeat ( NULL, ctx, 0, & nextInBlob, & inRef, NULL, NULL ) );
    REQUIRE_FAILED ();

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_FindRepeat_OffsetOutOfRange, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    uint64_t nextInBlob;
    uint64_t inRef;
    REQUIRE ( ! NGS_ReferenceBlobFindRepeat ( NULL, ctx, 20000, & nextInBlob, & inRef, NULL, NULL ) );
    REQUIRE_FAILED ();

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_FindRepeat_NullOutputParam_1, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession_WithRepeats, 1 );

    uint64_t inRef;
    REQUIRE ( ! NGS_ReferenceBlobFindRepeat ( m_blob, ctx, 0, NULL, & inRef, NULL, NULL ) );
    REQUIRE_FAILED ();

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_FindRepeat_NullOutputParam_2, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession_WithRepeats, 1 );

    uint64_t nextInBlob;
    REQUIRE ( ! NGS_ReferenceBlobFindRepeat ( m_blob, ctx, 0, & nextInBlob, NULL, NULL, NULL ) );
    REQUIRE_FAILED ();

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_FindRepeat_NotFound, ReferenceBlobFixture )
{
    ENTRY;
    GetBlob ( CSRA1_Accession, 1 );

    uint64_t nextInBlob;
    uint64_t inRef;
    REQUIRE ( ! NGS_ReferenceBlobFindRepeat ( m_blob, ctx, 0, & nextInBlob, & inRef, NULL, NULL ) );

    EXIT;
}


FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_FindRepeat_FoundFirst, ReferenceBlobFixture )
{
    ENTRY;
    const int64_t repeatedRowId = 96;
    GetBlob ( CSRA1_Accession_WithRepeats, repeatedRowId ); /* this blob consists of 9 repeated all-N rows */
    int64_t firstRow;
    uint64_t rowCount;
    NGS_ReferenceBlobRowRange ( m_blob, ctx, &firstRow, &rowCount );
    REQUIRE_EQ ( (int64_t)repeatedRowId, firstRow );
    REQUIRE_EQ ( (uint64_t)9, rowCount );

    uint64_t nextInBlob;
    uint64_t inRef;
    uint32_t repeatCount;
    uint64_t increment;
    REQUIRE ( NGS_ReferenceBlobFindRepeat ( m_blob, ctx, 0, & nextInBlob, & inRef, & repeatCount, & increment ) );
    REQUIRE_EQ ( (uint64_t)0, nextInBlob );
    REQUIRE_EQ ( (uint64_t)( repeatedRowId - 1 ) * 5000, inRef );
    REQUIRE_EQ ( (uint32_t)rowCount, repeatCount );
    REQUIRE_EQ ( (uint64_t)5000, increment );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobMake_FindRepeat_FoundNext, ReferenceBlobFixture )
{
    ENTRY;
    GetCursor ( CSRA1_Accession_WithRepeats );

    int64_t row=37;
    while ( row < 620410 )
    {
        m_blob = NGS_ReferenceBlobMake ( m_ctx, m_curs, row );
        if ( FAILED () )
        {
            CLEAR ();
            break;
        }
        //NGS_ReferenceBlobPrintPageMap(m_blob, m_ctx);
        int64_t firstRow;
        uint64_t rowCount;
        NGS_ReferenceBlobRowRange ( m_blob, ctx, &firstRow, &rowCount );
        if ( rowCount > 1 )
        {
            const bool PrintOn = false;
            bool first = true;
            uint64_t nextInBlob = 0;
            uint64_t inRef;
            uint32_t repeatCount;
            uint64_t increment;
            while ( NGS_ReferenceBlobFindRepeat ( m_blob, ctx, nextInBlob, & nextInBlob, & inRef, & repeatCount, & increment ) )
            {
                if ( PrintOn && first )
                {
                    cout << firstRow << ", " << rowCount << ": ";
                    first = false;
                }
                if ( repeatCount <= 1 )
                {
                    cout << nextInBlob << "(row=" << (inRef/5000+1) << ", rep=" << repeatCount << ") !" << endl;
                    FAIL("");
                }
                if ( increment == 0 )
                {
                    cout << nextInBlob << "(row=" << (inRef/5000+1) << ", rep=" << repeatCount << "), increment=0 !" << endl;
                    FAIL("");
                }

                if ( PrintOn )
                {
                    cout << nextInBlob << "(row=" << ( inRef / increment + 1 ) << ", rep=" << repeatCount << "), " << endl;
                }

                REQUIRE_EQ ( (uint64_t)5000, increment );

                {
                    int64_t rowId = inRef / increment + 1;
                    const void *base1;
                    NGS_CursorCellDataDirect ( m_curs, ctx, rowId, reference_READ, NULL, &base1, NULL, NULL );
                    const void *base2;
                    NGS_CursorCellDataDirect ( m_curs, ctx, rowId + 1, reference_READ, NULL, &base2, NULL, NULL );
                    REQUIRE_EQ ( 0, memcmp ( base1, base2, increment ) );
                }

                nextInBlob += increment;
            }
            if ( PrintOn && ! first )
            {
                cout << endl;
            }
        }
        row = firstRow + rowCount;

        NGS_ReferenceBlobRelease ( m_blob, m_ctx );
    }

    EXIT;
}

#if 0


//////////////////////////////////////////// NGS_ReferenceBlobIterator

class BlobIteratorFixture : public NGS_C_Fixture
{
public:
    BlobIteratorFixture ()
    :   m_tbl ( 0 ),
        m_blobIt ( 0 )
    {
    }

    void MakeSRA( const char* acc )
    {
        if ( m_tbl != 0 )
            VTableRelease ( m_tbl );
        if ( VDBManagerOpenTableRead ( m_ctx -> rsrc -> vdb, & m_tbl, NULL, acc ) != 0 )
            throw logic_error ("BlobIteratorFixture::MakeSRA VDBManagerOpenTableRead failed");
    }
    void MakeIterator( const char* acc )
    {
        MakeSRA ( acc );
        NGS_String* run = NGS_StringMake ( m_ctx, acc, string_size ( acc ) );
        m_blobIt = NGS_ReferenceBlobIteratorMake ( m_ctx, run, m_tbl );
        NGS_StringRelease ( run, m_ctx );
    }
    virtual void Release()
    {
        if (m_ctx != 0)
        {
            if ( m_blobIt != 0 )
            {
                NGS_ReferenceBlobIteratorRelease ( m_blobIt, m_ctx );
            }
            if ( m_tbl != 0 )
            {
                VTableRelease ( m_tbl );
            }
        }
        NGS_C_Fixture :: Release ();
    }

    const VTable* m_tbl;
    struct NGS_ReferenceBlobIterator* m_blobIt;
};

FIXTURE_TEST_CASE ( NGS_ReferenceBlobIterator_BadMake, BlobIteratorFixture )
{
    ENTRY;

    NGS_String* run = NGS_StringMake ( m_ctx, "", 0 );
    REQUIRE ( ! FAILED () );

    struct NGS_ReferenceBlobIterator* blobIt = NGS_ReferenceBlobIteratorMake ( ctx, run, NULL );
    REQUIRE_FAILED ();
    REQUIRE_NULL ( blobIt );
    NGS_StringRelease ( run, ctx );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobIterator_CreateRelease, BlobIteratorFixture )
{
    ENTRY;
    MakeSRA ( SRA_Accession );

    NGS_String* run = NGS_StringMake ( m_ctx, SRA_Accession, string_size ( SRA_Accession ) );
    REQUIRE ( ! FAILED () );
    struct NGS_ReferenceBlobIterator* blobIt = NGS_ReferenceBlobIteratorMake ( m_ctx, run, m_tbl );
    REQUIRE ( ! FAILED () );
    NGS_StringRelease ( run, m_ctx );
    REQUIRE_NOT_NULL ( blobIt );
    REQUIRE ( ! FAILED () );
    // Release
    NGS_ReferenceBlobIteratorRelease ( blobIt, m_ctx );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobIterator_DuplicateRelease, BlobIteratorFixture )
{
    ENTRY;
    MakeIterator ( SRA_Accession );

    // Duplicate
    struct NGS_ReferenceBlobIterator* anotherBlobIt = NGS_ReferenceBlobIteratorDuplicate ( m_blobIt, m_ctx );
    REQUIRE_NOT_NULL ( anotherBlobIt );
    // Release
    NGS_ReferenceBlobIteratorRelease ( anotherBlobIt, m_ctx );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobIterator_Next, BlobIteratorFixture )
{
    ENTRY;
    MakeIterator ( SRA_Accession );

    struct NGS_ReferenceBlob* blob = NGS_ReferenceBlobIteratorNext ( m_blobIt, m_ctx );
    REQUIRE ( ! FAILED () );
    REQUIRE_NOT_NULL ( blob );
    NGS_ReferenceBlobRelease ( blob, ctx );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobIterator_HasMore, BlobIteratorFixture )
{
    ENTRY;
    MakeIterator ( SRA_Accession );

    REQUIRE ( NGS_ReferenceBlobIteratorHasMore ( m_blobIt, m_ctx ) );
    REQUIRE ( ! FAILED () );

    EXIT;
}

#if VDB_3075_has_been_fixed
FIXTURE_TEST_CASE ( NGS_ReferenceBlobIterator_FullScan, BlobIteratorFixture )
{
    ENTRY;
    MakeIterator ( SRA_Accession );

    uint32_t count = 0;
    while ( NGS_ReferenceBlobIteratorHasMore ( m_blobIt, m_ctx ) )
    {
        struct NGS_ReferenceBlob* blob = NGS_ReferenceBlobIteratorNext ( m_blobIt, m_ctx );
        REQUIRE_NOT_NULL ( blob );
        NGS_ReferenceBlobRelease ( blob, ctx );
        ++count;
    }
    REQUIRE_EQ ( (uint32_t)243, count);
    REQUIRE_NULL ( NGS_ReferenceBlobIteratorNext ( m_blobIt, m_ctx ) );

    EXIT;
}
#endif

FIXTURE_TEST_CASE ( NGS_ReferenceBlobIterator_SparseTable, BlobIteratorFixture )
{
    ENTRY;
    MakeIterator ( "./data/SparseReferenceBlobs" );
    // only row 1 and 10 are present

    REQUIRE ( NGS_ReferenceBlobIteratorHasMore ( m_blobIt, m_ctx ) );
    {
        struct NGS_ReferenceBlob* blob = NGS_ReferenceBlobIteratorNext ( m_blobIt, m_ctx );
        REQUIRE_NOT_NULL ( blob );
        int64_t first = 0;
        uint64_t count = 0;
        NGS_ReferenceBlobRowRange ( blob, m_ctx, & first, & count );
        REQUIRE_EQ ( (int64_t)1, first );
        REQUIRE_EQ ( (uint64_t)1, count );
        NGS_ReferenceBlobRelease ( blob, ctx );
    }

    REQUIRE ( NGS_ReferenceBlobIteratorHasMore ( m_blobIt, m_ctx ) );
    {
        struct NGS_ReferenceBlob* blob = NGS_ReferenceBlobIteratorNext ( m_blobIt, m_ctx );
        REQUIRE_NOT_NULL ( blob );
        int64_t first = 0;
        uint64_t count = 0;
        NGS_ReferenceBlobRowRange ( blob, m_ctx, & first, & count );
        REQUIRE_EQ ( (int64_t)10, first );
        REQUIRE_EQ ( (uint64_t)1, count );
        NGS_ReferenceBlobRelease ( blob, ctx );
    }

    REQUIRE ( ! NGS_ReferenceBlobIteratorHasMore ( m_blobIt, m_ctx ) );

    EXIT;
}

FIXTURE_TEST_CASE ( NGS_ReferenceBlobIterator_IteratorRetreats, BlobIteratorFixture )
{   // VDB-2809: NGS_ReferenceBlobIterator returns overlapping blobs on CSRA1 accessions
    ENTRY;
    const char* acc = "SRR833251";
    const VDatabase *db;
    REQUIRE_RC ( VDBManagerOpenDBRead ( m_ctx -> rsrc -> vdb, & db, NULL, acc ) );
    REQUIRE_RC ( VDatabaseOpenTableRead ( db, & m_tbl, "SEQUENCE" ) );
    REQUIRE_RC ( VDatabaseRelease ( db ) );

    NGS_String* run = NGS_StringMake ( m_ctx, acc, string_size ( acc ) );
    m_blobIt = NGS_ReferenceBlobIteratorMake ( m_ctx, run, m_tbl );
    NGS_StringRelease ( run, m_ctx );

    int64_t rowId = 1;
    while (true)
    {
        struct NGS_ReferenceBlob* blob = NGS_ReferenceBlobIteratorNext ( m_blobIt, m_ctx );
        if ( blob == 0 )
        {
            break;
        }
        int64_t first = 0;
        uint64_t count = 0;
        NGS_ReferenceBlobRowRange ( blob, m_ctx, & first, & count );
        REQUIRE_EQ ( rowId, first );
        NGS_ReferenceBlobRelease ( blob, ctx );
        rowId = first + count;
    }

    EXIT;
}
#endif
//////////////////////////////////////////// Main

extern "C"
{

#include <kapp/args.h>

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

const char UsageDefaultName[] = "test-ngs";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t m_coll=NgsReferenceBlobTestSuite(argc, argv);
    return m_coll;
}

}

