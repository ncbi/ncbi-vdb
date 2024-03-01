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

#include "VDB_Fixture.hpp"

#include <vdb/manager.h> // VDBManager

using namespace std;

TEST_SUITE( VdbTextSuite )

TEST_CASE( VdbMgr )
{
KDB_EXTERN rc_t CC KDBManagerMakeText ( const KDBManager **mgr, const char * input, char * error, size_t error_size );

    const VDBManager *mgr = NULL;
    rc_t rc = VDBManagerMakeRead(NULL, NULL);
    if (rc == 0)
        FAIL( "FAIL: VDBManagerMakeRead(NULL) succeed" );
}

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

const char UsageDefaultName[] = "test-vdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=VdbTextSuite(argc, argv);
    return rc;
}

}
