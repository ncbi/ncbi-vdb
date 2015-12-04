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

// allow g++ to find INT64_MAX in stdint.h
#define __STDC_LIMIT_MACROS

#include <ktst/unit_test.hpp>

#include <klib/log.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <kdb/rowset.h>

#include <stdlib.h>
#include <time.h>

#include <vector>
#include <set>

TEST_SUITE(KRowSetTestSuite);

void vector_inserter ( int64_t row_id, void *data )
{
    std::vector<int64_t> * rows = (std::vector<int64_t> *) data;
    rows->push_back ( row_id );
}

class RowSetFixture
{
public:
    int64_t GenerateId ( int64_t range_start, int64_t range_count )
    {
        if ( range_start == -1 )
            range_start = 0;

        if ( range_count == -1 || range_start + range_count < 0 )
            range_count = INT64_MAX - range_start;

        int64_t generated_id = ((int64_t)rand() << 32) | rand();
        generated_id &= INT64_MAX; // make sure it is positive

        generated_id = generated_id % range_count;
        generated_id += range_start;

        return generated_id;
    }

    void RunChecks ( const KRowSet * rowset, std::set<int64_t> & inserted_rows_set )
    {
        RunChecksInt ( rowset, inserted_rows_set, false );
        RunChecksInt ( rowset, inserted_rows_set, true );
    }
private:
    void RunChecksInt ( const KRowSet * rowset, std::set<int64_t> & inserted_rows_set, bool reverse_walk )
    {
        std::vector<int64_t> inserted_rows;
        std::vector<int64_t> returned_rows;
        uint64_t num_rows;

        if ( !reverse_walk )
            std::copy(inserted_rows_set.begin(), inserted_rows_set.end(), std::back_inserter(inserted_rows));
        else
            std::copy(inserted_rows_set.rbegin(), inserted_rows_set.rend(), std::back_inserter(inserted_rows));

        THROW_ON_RC ( KRowSetWalkRows( rowset, reverse_walk, vector_inserter, (void *)&returned_rows ) );

        THROW_ON_RC ( KRowSetGetNumRows ( rowset, &num_rows ) );

        if ( inserted_rows.size() != returned_rows.size() )
            FAIL("inserted_rows.size() != returned_rows.size()");
        if ( num_rows != returned_rows.size() )
            FAIL("num_rows != returned_rows.size()");
        if ( inserted_rows != returned_rows )
            FAIL("inserted_rows != returned_rows");
    }
};



FIXTURE_TEST_CASE ( KRowSetScatteredRows, RowSetFixture )
{
	KRowSet * rowset;
	std::set<int64_t> inserted_rows_set;

	REQUIRE_RC ( KCreateRowSet ( &rowset ) );
	for ( int i = 0; i < 10000; ++i )
	{
		int64_t row_id = GenerateId ( -1, -1 );
		if ( inserted_rows_set.find( row_id ) ==  inserted_rows_set.end() )
		{
			REQUIRE_RC ( KRowSetInsertRow ( rowset, row_id ) );
			inserted_rows_set.insert( row_id );
		}
		else
			--i;
	}

	RunChecks ( rowset, inserted_rows_set );
	REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetDenseRows, RowSetFixture )
{
	KRowSet * rowset;
	std::set<int64_t> inserted_rows_set;

	REQUIRE_RC ( KCreateRowSet ( &rowset ) );
	for ( int i = 0; i < 10000; ++i )
	{
		int64_t row_id = GenerateId ( 0, 131072 ); // row ids will only go to first two leaves
		if ( inserted_rows_set.find( row_id ) ==  inserted_rows_set.end() )
		{
			REQUIRE_RC ( KRowSetInsertRow ( rowset, row_id ) );
			inserted_rows_set.insert( row_id );
		}
		else
			--i;
	}

	RunChecks ( rowset, inserted_rows_set );
	REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetSerialRows, RowSetFixture )
{
    KRowSet * rowset;
    std::set<int64_t> inserted_rows_set;

    REQUIRE_RC ( KCreateRowSet ( &rowset ) );
    for ( int i = 0; i < 100000; ++i )
    {
        int64_t row_id = i; // row ids will only go to first two leaves
        REQUIRE_RC ( KRowSetInsertRow ( rowset, row_id ) );
        inserted_rows_set.insert( row_id );
    }

    RunChecks ( rowset, inserted_rows_set );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetRowRanges, RowSetFixture )
{
    KRowSet * rowset;
    std::set<int64_t> inserted_rows_set;

    int row_ids[] = { 0, 5, 1, 6, 20, 10, 55, 60, 65, 70, 75, 80, 85, 999,  2001 };
    int counts[]  = { 1, 1, 4, 4, 10, 10, 1,  1,  1,  1,  1,  1,  1,  1000, 1000 };

    REQUIRE_RC ( KCreateRowSet ( &rowset ) );
    for ( int i = 0; i < sizeof row_ids / sizeof row_ids[0]; ++i )
    {
        int64_t row_id = row_ids[i];
        int count = counts[i];

        REQUIRE_RC ( KRowSetInsertRowRange ( rowset, row_id, count, NULL ) );
        for ( int j = 0; j < count; ++j )
            inserted_rows_set.insert( row_id + j );
    }

    RunChecks ( rowset, inserted_rows_set );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetRowRangesOverlapDuplicates, RowSetFixture )
{
    KRowSet * rowset;
    std::set<int64_t> inserted_rows_set;

    int row_ids[] = { 5, 10 };
    int counts[]  = { 1, 5    };

    int overlap_row_ids[] = { 0, 5, 5, 2, 9, 9, 14 };
    int overlap_counts[]  = { 6, 1, 2, 6, 2, 10, 2 };

    REQUIRE_RC ( KCreateRowSet ( &rowset ) );
    for ( int i = 0; i < sizeof row_ids / sizeof row_ids[0]; ++i )
    {
        int64_t row_id = row_ids[i];
        int count = counts[i];

        REQUIRE_RC ( KRowSetInsertRowRange ( rowset, row_id, count, NULL ) );
        for ( int j = 0; j < count; ++j )
            inserted_rows_set.insert( row_id + j );
    }

    for ( int i = 0; i < sizeof overlap_row_ids / sizeof overlap_row_ids[0]; ++i )
    {
        int64_t row_id = overlap_row_ids[i];
        int count = overlap_counts[i];
        uint64_t inserted;

        REQUIRE_RC_FAIL ( KRowSetInsertRowRange ( rowset, row_id, count, &inserted ) );
        for ( int j = 0; j < inserted; ++j )
            inserted_rows_set.insert( row_id + j );
    }

    RunChecks ( rowset, inserted_rows_set );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetRowRangesDenseOverlapDuplicates, RowSetFixture )
{
    KRowSet * rowset;
    std::set<int64_t> inserted_rows_set;

    int row_ids[] = { 0, 5, 10, 15, 20, 25, 30, 35, 40, 1000 };
    int counts[]  = { 1, 1, 1,  1,  1,  1,  1,  1,  1,  1    };

    int overlap_row_ids[] = { 500 };
    int overlap_counts[]  = { 1000 };

    REQUIRE_RC ( KCreateRowSet ( &rowset ) );
    for ( int i = 0; i < sizeof row_ids / sizeof row_ids[0]; ++i )
    {
        int64_t row_id = row_ids[i];
        int count = counts[i];

        REQUIRE_RC ( KRowSetInsertRowRange ( rowset, row_id, count, NULL ) );
        for ( int j = 0; j < count; ++j )
            inserted_rows_set.insert( row_id + j );
    }

    for ( int i = 0; i < sizeof overlap_row_ids / sizeof overlap_row_ids[0]; ++i )
    {
        int64_t row_id = overlap_row_ids[i];
        int count = overlap_counts[i];
        uint64_t inserted;

        REQUIRE_RC_FAIL ( KRowSetInsertRowRange ( rowset, row_id, count, &inserted ) );
        for ( int j = 0; j < inserted; ++j )
            inserted_rows_set.insert( row_id + j );
    }

    RunChecks ( rowset, inserted_rows_set );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/main.h>
#include <kapp/args.h>

ver_t CC KAppVersion ( void )
{
    return 0;
}


const char UsageDefaultName[] = "test-rowset";

rc_t CC UsageSummary ( const char *progname )
{
    return KOutMsg ( "\n"
                     "Usage:\n"
                     "  %s [Options] <target>\n"
                     "\n"
                     "Summary:\n"
                     "  test the rowset.\n"
                     , progname
        );
}

rc_t CC Usage ( const Args *args )
{
    const char * progname = UsageDefaultName;
    const char * fullpath = UsageDefaultName;
    rc_t rc;

    if (args == NULL)
        rc = RC (rcApp, rcArgv, rcAccessing, rcSelf, rcNull);
    else
        rc = ArgsProgram (args, &fullpath, &progname);
    if (rc)
        progname = fullpath = UsageDefaultName;

    UsageSummary (progname);

    KOutMsg ("Options:\n");

    HelpOptionsStandard ();

    HelpVersion (fullpath, KAppVersion());

    return rc;
}
rc_t CC KMain ( int argc, char *argv [] )
{
    srand ( time(NULL) );
	return KRowSetTestSuite(argc, argv);
}

}

