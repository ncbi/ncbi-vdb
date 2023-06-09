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

/* A test project to make sure C++ standard library is not required for VDB to function */

#include <klib/printf.h>

#include <vdb/manager.h> // VDBManager
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>

#include <kfg/config.h>

#include <sysalloc.h>
#include <stdio.h>

#define CHECK_RC(call) { rc_t rc = call; if (rc ) return rc; }

rc_t
TestVdb ( const char * p_acc, const char * p_columns[])
{   /* open and read an accession */
    const VDBManager * mgr;
    const VTable * tbl;
    const VCursor * curs;

    CHECK_RC ( VDBManagerMakeRead ( & mgr, NULL) );
    CHECK_RC ( VDBManagerOpenTableRead ( mgr, & tbl, NULL, p_acc ) );
    CHECK_RC ( VTableCreateCursorRead ( tbl, & curs ) );
    {
        int i;
        for ( i = 0; p_columns [ i ] != NULL; ++i )
        {
            uint32_t ignore;
            CHECK_RC ( VCursorAddColumn ( curs, &ignore, p_columns[i] ) );
        }
        CHECK_RC ( VCursorOpen(curs) );
    }

    CHECK_RC ( VTableRelease ( tbl ) );
    CHECK_RC ( VCursorRelease ( curs ) );
    CHECK_RC ( VDBManagerRelease ( mgr ) );

    return 0;
}

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

const char UsageDefaultName[] = "test-link";

rc_t CC KMain ( int argc, char *argv [] )
{
    const char * columns [] = { "READ_LEN", NULL };
    rc_t rc;
    KConfigDisableUserSettings();
    rc = TestVdb ( "SRR000123", columns );
    if ( rc == 0 )
    {
        printf("Link test passed.\n");
    }
    else
    {
        char msg[1024];
        string_printf ( msg, sizeof msg, NULL, "Link test failed, rc = %R", rc );
        printf("%s\n", msg);
    }
    return rc;
}

