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

    /*
     * "density" - can is a number between -1 and 8.
     *   -1 means that most probably each row will go to a separate leaf
     *   0 means that rows might go to same leaves, but those leaves should still not be dense (and store data in array of ranges, not in bitmap)
     *   from 1 to 8 change density level:
     *   - 1 means that every (up to) 8k rows go to a each leaf
     *   - 8 means that every (up to) 65k rows (maximum rows per leaf) will go to each leaf
     */
    std::set<int64_t> InsertRandomRows ( KRowSet * rowset, int num_rows, int density )
    {
        int64_t range_start = 0;
        int64_t range_count;
        std::set<int64_t> inserted_rows_set;

        assert ( num_rows > 0 );
        assert ( density >= -1 );
        assert ( density <= 8 );

        if ( density == -1 )
            range_count = -1;
        else if ( density == 0 )
            range_count = ((num_rows - 1) / 5 + 1) * 65536;
        else
        {
            assert ( num_rows > 8 );
            range_count = ((num_rows - 1) / (8192 * density) + 1) * 65536;
        }

        for ( int i = 0; i < num_rows; ++i )
        {
            int64_t row_id = GenerateId ( range_start, range_count );
            if ( inserted_rows_set.find( row_id ) ==  inserted_rows_set.end() )
            {
                bool inserted;
                THROW_ON_RC ( KRowSetAddRowId ( rowset, row_id, &inserted ) );
                if ( !inserted )
                    FAIL("Failed to insert a row");
                inserted_rows_set.insert( row_id );
            }
            else
                --i;
        }

        return inserted_rows_set;
    }

    void RunChecks ( const KRowSet * rowset, std::set<int64_t> & inserted_rows_set )
    {
        RunChecksInt ( rowset, inserted_rows_set, false );
        RunChecksInt ( rowset, inserted_rows_set, true );
    }

    std::set<int64_t> SetIntersection ( const std::set<int64_t>& set1, const std::set<int64_t>& set2 )
    {
        std::set<int64_t>::iterator first1 = set1.begin(),
                                    first2 = set2.begin(),
                                    last1 = set1.end(),
                                    last2 = set2.end();
        std::set<int64_t> result;

        while (first1 != last1 && first2 != last2) {
            if (*first1 < *first2) {
                ++first1;
            } else  {
                if (!(*first2 < *first1)) {
                    result.insert( *first1++ );
                }
                ++first2;
            }
        }
        return result;
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

        THROW_ON_RC ( KRowSetVisit ( rowset, reverse_walk, vector_inserter, (void *)&returned_rows ) );

        THROW_ON_RC ( KRowSetGetNumRowIds ( rowset, &num_rows ) );

/*
        // useful in debugging
        KOutMsg("inserted:\n");
        for ( int i = 0; i < inserted_rows.size(); ++i )
        {
            KOutMsg ( "%d, ", inserted_rows[i] );
        }
        KOutMsg("\n");
        KOutMsg("returned:\n");
        for ( int i = 0; i < returned_rows.size(); ++i )
        {
            KOutMsg ( "%d, ", returned_rows[i] );
        }
        KOutMsg("\n");
*/
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

    REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset ) );

    inserted_rows_set = InsertRandomRows ( rowset, 10000, -1 );

    RunChecks ( rowset, inserted_rows_set );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetDenseRows, RowSetFixture )
{
    KRowSet * rowset;
    std::set<int64_t> inserted_rows_set;

    REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset ) );

    inserted_rows_set = InsertRandomRows ( rowset, 10000, 1 );

    RunChecks ( rowset, inserted_rows_set );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetSerialRows, RowSetFixture )
{
    KRowSet * rowset;
    std::set<int64_t> inserted_rows_set;

    REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset ) );
    for ( int i = 0; i < 100000; ++i )
    {
        int64_t row_id = i; // row ids will only go to first two leaves
        bool inserted;
        REQUIRE_RC ( KRowSetAddRowId ( rowset, row_id, &inserted ) );
        REQUIRE ( inserted );
        inserted_rows_set.insert( row_id );
    }

    RunChecks ( rowset, inserted_rows_set );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetRowRanges, RowSetFixture )
{
    KRowSet * rowset;
    std::set<int64_t> inserted_rows_set;

    int64_t row_ids[] = { 0, 5, 1, 6, 20, 10, 55, 60, 65, 70, 75, 80, 85, 999,  2001 };
    uint64_t counts[]  = { 1, 1, 4, 4, 10, 10, 1,  1,  1,  1,  1,  1,  1,  1000, 1000 };

    REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset ) );
    for ( int i = 0; i < sizeof row_ids / sizeof row_ids[0]; ++i )
    {
        int64_t row_id = row_ids[i];
        uint64_t count = counts[i];
        uint64_t inserted;

        REQUIRE_RC ( KRowSetAddRowIdRange ( rowset, row_id, count, &inserted ) );
        for ( int j = 0; j < count; ++j )
            inserted_rows_set.insert( row_id + j );

        REQUIRE_EQ ( count, inserted );
    }

    RunChecks ( rowset, inserted_rows_set );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetRowRangesOverlapDuplicates, RowSetFixture )
{
    KRowSet * rowset;
    std::set<int64_t> inserted_rows_set;

    int64_t row_ids[] = { 5, 10 };
    uint64_t counts[]  = { 1, 5    };

    int64_t overlap_row_ids[] = { 0, 5, 5, 2, 9, 9, 14 };
    uint64_t overlap_counts[]  = { 6, 1, 2, 6, 2, 10, 2 };

    REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset ) );
    for ( int i = 0; i < sizeof row_ids / sizeof row_ids[0]; ++i )
    {
        int64_t row_id = row_ids[i];
        uint64_t count = counts[i];
        uint64_t inserted;

        REQUIRE_RC ( KRowSetAddRowIdRange ( rowset, row_id, count, &inserted ) );
        for ( int j = 0; j < count; ++j )
            inserted_rows_set.insert( row_id + j );

        REQUIRE_EQ ( count, inserted );
    }

    for ( int i = 0; i < sizeof overlap_row_ids / sizeof overlap_row_ids[0]; ++i )
    {
        int64_t row_id = overlap_row_ids[i];
        uint64_t count = overlap_counts[i];
        uint64_t inserted;
        uint64_t inserted_set_size = inserted_rows_set.size();

        REQUIRE_RC ( KRowSetAddRowIdRange ( rowset, row_id, count, &inserted ) );
        for ( int j = 0; j < count; ++j )
            inserted_rows_set.insert( row_id + j );

        REQUIRE ( inserted_rows_set.size() - inserted_set_size == inserted );
    }

    RunChecks ( rowset, inserted_rows_set );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetRowRangesDenseOverlapDuplicates, RowSetFixture )
{
    KRowSet * rowset;
    std::set<int64_t> inserted_rows_set;

    int64_t row_ids[] = { 0, 5, 10, 15, 20, 25, 30, 35, 40, 1000 };
    uint64_t counts[]  = { 1, 1, 1,  1,  1,  1,  1,  1,  1,  1    };

    int64_t overlap_row_ids[] = { 500 };
    uint64_t overlap_counts[]  = { 1000 };

    REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset ) );
    for ( int i = 0; i < sizeof row_ids / sizeof row_ids[0]; ++i )
    {
        int64_t row_id = row_ids[i];
        uint64_t count = counts[i];
        uint64_t inserted;

        REQUIRE_RC ( KRowSetAddRowIdRange ( rowset, row_id, count, &inserted ) );
        for ( int j = 0; j < count; ++j )
            inserted_rows_set.insert( row_id + j );

        REQUIRE_EQ ( count, inserted );
    }

    for ( int i = 0; i < sizeof overlap_row_ids / sizeof overlap_row_ids[0]; ++i )
    {
        int64_t row_id = overlap_row_ids[i];
        uint64_t count = overlap_counts[i];
        uint64_t inserted;
        uint64_t inserted_set_size = inserted_rows_set.size();

        REQUIRE_RC ( KRowSetAddRowIdRange ( rowset, row_id, count, &inserted ) );
        for ( int j = 0; j < count; ++j )
            inserted_rows_set.insert( row_id + j );

        REQUIRE ( inserted_rows_set.size() - inserted_set_size == inserted );
    }

    RunChecks ( rowset, inserted_rows_set );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetIteratorOutOfBoundaries, RowSetFixture )
{
    const int move_out_boundaries = 2;

    KRowSet * rowset;

    int64_t row_id_inserted = 55;

    REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset ) );
    REQUIRE_RC ( KRowSetAddRowIdRange ( rowset, row_id_inserted, 1, NULL ) );

    uint64_t num_rows;
    REQUIRE_RC ( KRowSetGetNumRowIds ( rowset, &num_rows ) );
    REQUIRE_EQ ( num_rows, (uint64_t)1 );

    int64_t row_id_retrieved;
    KRowSetIterator * it;
    REQUIRE_RC ( KRowSetMakeIterator ( rowset, &it ) );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
    rowset = NULL;

    REQUIRE ( KRowSetIteratorIsValid ( it ) );
    REQUIRE_RC ( KRowSetIteratorRowId ( it, &row_id_retrieved ) );
    REQUIRE_EQ ( row_id_inserted, row_id_retrieved );

    // move forward out of boundaries and then move back
    for ( int i = 0; i < move_out_boundaries; ++i )
    {
        REQUIRE_RC_FAIL ( KRowSetIteratorNext( it ) );
        REQUIRE ( !KRowSetIteratorIsValid ( it ) );
    }
    for ( int i = move_out_boundaries - 1; i >= 0; --i )
    {
        rc_t rc = KRowSetIteratorPrev( it );
        if ( i == 0 )
        {
            REQUIRE_RC ( rc );
        }
        else
        {
            REQUIRE_RC_FAIL ( rc );
            REQUIRE ( !KRowSetIteratorIsValid ( it ) );
        }
    }
    REQUIRE ( KRowSetIteratorIsValid ( it ) );
    REQUIRE_RC ( KRowSetIteratorRowId ( it, &row_id_retrieved ) );
    REQUIRE_EQ ( row_id_inserted, row_id_retrieved );

    // move backward out of boundaries and then move back
    for ( int i = 0; i < move_out_boundaries; ++i )
    {
        REQUIRE_RC_FAIL ( KRowSetIteratorPrev( it ) );
        REQUIRE ( !KRowSetIteratorIsValid ( it ) );
    }
    for ( int i = move_out_boundaries - 1; i >= 0; --i )
    {
        rc_t rc = KRowSetIteratorNext( it );
        if ( i == 0 )
        {
            REQUIRE_RC ( rc );
        }
        else
        {
            REQUIRE_RC_FAIL ( rc );
            REQUIRE ( !KRowSetIteratorIsValid ( it ) );
        }
    }
    REQUIRE ( KRowSetIteratorIsValid ( it ) );
    REQUIRE_RC ( KRowSetIteratorRowId ( it, &row_id_retrieved ) );
    REQUIRE_EQ ( row_id_inserted, row_id_retrieved );

    REQUIRE_RC ( KRowSetIteratorRelease ( it ) );

}

FIXTURE_TEST_CASE ( KRowSetIteratorMoveForwardAndBackward, RowSetFixture )
{
    KRowSet * rowset;

    int64_t row_ids[] = { 0, 5, 10, 15, 20, 25, 30, 35, 40, 1000, 100000 };

    REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset ) );
    for ( int i = 0; i < sizeof row_ids / sizeof row_ids[0]; ++i )
    {
        int64_t row_id = row_ids[i];
        bool inserted;

        REQUIRE_RC ( KRowSetAddRowId ( rowset, row_id, &inserted ) );
        REQUIRE ( inserted );
    }

    uint64_t num_rows;
    REQUIRE_RC ( KRowSetGetNumRowIds ( rowset, &num_rows ) );
    REQUIRE_EQ ( num_rows, (uint64_t)sizeof row_ids / sizeof row_ids[0] );

    KRowSetIterator * it;
    REQUIRE_RC ( KRowSetMakeIterator ( rowset, &it ) );
    REQUIRE_RC ( KRowSetRelease( rowset ) );
    rowset = NULL;

    // move forward
    for ( int i = 0; i < sizeof row_ids / sizeof row_ids[0]; ++i )
    {
        int64_t row_id;

        if ( i != 0 )
            REQUIRE_RC ( KRowSetIteratorNext( it ) );

        REQUIRE ( KRowSetIteratorIsValid ( it ) );
        REQUIRE_RC ( KRowSetIteratorRowId ( it, &row_id ) );
        REQUIRE_EQ ( row_id, row_ids[i] );
    }

    // move backward
    for ( int i = sizeof row_ids / sizeof row_ids[0] - 1; i >= 0; --i )
    {
        int64_t row_id;

        if ( i != sizeof row_ids / sizeof row_ids[0] - 1 )
            REQUIRE_RC ( KRowSetIteratorPrev( it ) );

        REQUIRE ( KRowSetIteratorIsValid ( it ) );
        REQUIRE_RC ( KRowSetIteratorRowId ( it, &row_id ) );
        REQUIRE_EQ ( row_id, row_ids[i] );
    }

    REQUIRE_RC ( KRowSetIteratorRelease ( it ) );
}

FIXTURE_TEST_CASE ( KRowSetIteratorOverEmptySet, RowSetFixture )
{
    KRowSet * rowset;
    REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset ) );

    KRowSetIterator * it;
    REQUIRE_RC_FAIL ( KRowSetMakeIterator ( rowset, &it ) );

    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

FIXTURE_TEST_CASE ( KRowSetOpAndSimpleTest, RowSetFixture )
{
    KRowSet * rowset1;
    KRowSet * rowset2;
    std::set<int64_t> inserted_rows_set1;
    std::set<int64_t> inserted_rows_set2;

    REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset1 ) );
    REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset2 ) );

    for ( int i = 0; i < 10000; ++i )
    {
        int64_t row_id = GenerateId ( -1, -1 );
        if ( inserted_rows_set1.find( row_id ) ==  inserted_rows_set1.end() )
        {
            bool inserted;
            REQUIRE_RC ( KRowSetAddRowId ( rowset1, row_id, &inserted ) );
            REQUIRE ( inserted );
            inserted_rows_set1.insert( row_id );
            // just don't insert a single row to a second rowset, but otherwise keep them equal
            if ( i != 0 )
            {
                REQUIRE_RC ( KRowSetAddRowId ( rowset2, row_id, &inserted ) );
                REQUIRE ( inserted );
                inserted_rows_set2.insert( row_id );
            }
        }
        else
            --i;
    }

    REQUIRE_RC ( KRowSetOpAnd ( rowset1, rowset2 ) );

    RunChecks ( rowset2, inserted_rows_set2 );
    RunChecks ( rowset1, inserted_rows_set2 );
    REQUIRE_RC ( KRowSetRelease( rowset1 ) );
    REQUIRE_RC ( KRowSetRelease( rowset2 ) );
}

FIXTURE_TEST_CASE ( KRowSetOpAndNormalTest, RowSetFixture )
{
    KRowSet * rowset1;
    KRowSet * rowset2;
    std::set<int64_t> inserted_rows_set1;
    std::set<int64_t> inserted_rows_set2;
    std::set<int64_t> intersection_set;

    // check all 4 combinations of dense/scattered leaves
    for ( int i = 0; i < 4; ++i )
    {
        REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset1 ) );
        REQUIRE_RC ( KTableMakeRowSet ( NULL, &rowset2 ) );

        inserted_rows_set1 = InsertRandomRows ( rowset1, 100000, (i & 1) ? 8 : 0 );
        inserted_rows_set2 = InsertRandomRows ( rowset2, 100000, (i & 2) ? 8 : 0 );

        RunChecks ( rowset1, inserted_rows_set1 );
        RunChecks ( rowset2, inserted_rows_set2 );

        REQUIRE_RC ( KRowSetOpAnd ( rowset1, rowset2 ) );

        intersection_set = SetIntersection ( inserted_rows_set1, inserted_rows_set2 );

        RunChecks ( rowset1, intersection_set );
        RunChecks ( rowset2, inserted_rows_set2 );

        REQUIRE_RC ( KRowSetOpAnd ( rowset2, rowset1 ) );
        RunChecks ( rowset1, intersection_set );
        RunChecks ( rowset2, intersection_set );

        REQUIRE_RC ( KRowSetRelease( rowset1 ) );
        REQUIRE_RC ( KRowSetRelease( rowset2 ) );
    }
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

