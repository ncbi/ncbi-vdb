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

#include <ktst/unit_test.hpp>

#include <klib/log.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <kdb/rowset.h>

#include <stdlib.h>
#include <time.h>

TEST_SUITE(KRowSetTestSuite);

TEST_CASE(KRowSet)
{
	rc_t rc;
	KRowSet * rowset;
	size_t num_rows;
	int i;

	srand ( time(NULL) );

	rc = KCreateRowSet ( &rowset );
	if ( rc != 0 )
		LOGERR ( klogInt, rc, "KCreateRowSet failed" );
	else
	{
#define insert(row_id) \
	rc = KRowSetInsertRow ( rowset, (row_id) ); \
	if ( rc != 0 )  LOGERR ( klogInt, rc, "KRowSetInsertRow failed" );

		for ( i = 0; i < 100; ++i )
		{
			int64_t row_id = ((int64_t)rand() << 32) | rand();
			insert ( row_id );
		}

#undef insert

		KRowSetPrintRowsByTraverse ( rowset );
		KRowSetPrintRowsByList ( rowset );

		KRowSetGetNumRows ( rowset, &num_rows );

		KOutMsg ( "Number of rows: %d\n", num_rows );

		KRowSetRelease( rowset );
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
	rc_t rc = KRowSetTestSuite(argc, argv);
    return rc;
}

}

