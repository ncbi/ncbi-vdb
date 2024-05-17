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
* Unit tests for mem-track module
*/

#include <ktst/unit_test.hpp>

#include "../libs/klib/mem-track.h"

#include <sstream>

using namespace std;

TEST_SUITE(KMemTrackSuite);

class MemTrackSuiteFixture
{
public:
    MemTrackSuiteFixture()
    {
        MemTrackInit();
    }

    const MemTrackBlockData * bd = nullptr;
};

FIXTURE_TEST_CASE ( KMemTrack_Empty, MemTrackSuiteFixture )
{
    REQUIRE_NULL( MemTrackGetBlock( nullptr ) );
}

FIXTURE_TEST_CASE ( KMemTrack_AllocNoFree, MemTrackSuiteFixture )
{
    const void * ptr = (const void*)1;
    MemTrackAlloc( ptr, 100 );
    bd = MemTrackGetBlock( ptr );
    REQUIRE_NOT_NULL( bd );
    REQUIRE_EQ( string(), bd->name );
    REQUIRE_NE( (clock_t)0, bd->created );
    REQUIRE_EQ( (clock_t)0, bd->freed );
    REQUIRE_EQ( (size_t)100, bd->max_size );
}

FIXTURE_TEST_CASE ( KMemTrack_AllocFree, MemTrackSuiteFixture )
{
    const void * ptr = (const void*)1;
    MemTrackAlloc( ptr, 100 );
    MemTrackFree( ptr );
    bd = MemTrackGetBlock( ptr );
    REQUIRE_NULL( bd );
}

FIXTURE_TEST_CASE ( KMemTrack_ReallocIncrease, MemTrackSuiteFixture )
{
    const void * ptr = (const void*)1;
    const void * new_ptr = (const void*)2;
    MemTrackAlloc( ptr, 100 );
    MemTrackRealloc( ptr, 200, new_ptr );
    REQUIRE_NULL( MemTrackGetBlock( ptr ) );
    bd = MemTrackGetBlock( new_ptr );
    REQUIRE_EQ( (size_t)200, bd->max_size );

    MemTrackFree( new_ptr );
    //MemTrackDigest( cout );
}

FIXTURE_TEST_CASE ( KMemTrack_ReallocDecrease, MemTrackSuiteFixture )
{
    const void * ptr = (const void*)1;
    MemTrackAlloc( ptr, 100 );
    MemTrackAlloc( ptr, 10 );
    bd = MemTrackGetBlock( ptr );
    REQUIRE_EQ( (size_t)100, bd->max_size );
}

FIXTURE_TEST_CASE ( KMemTrack_ReusePtr, MemTrackSuiteFixture )
{
    const void * ptr = (const void*)1;
    MemTrackAlloc( ptr, 100 );
    MemTrackFree( ptr );
    MemTrackAlloc( ptr, 10 );
    bd = MemTrackGetBlock( ptr );
    REQUIRE_EQ( (size_t)10, bd->max_size );
}

FIXTURE_TEST_CASE ( KMemTrack_Named, MemTrackSuiteFixture )
{
    const void * ptr = (const void*)1;
    MemTrackAlloc( ptr, 100 );
    const string name = "name";
    MemTrackName( ptr, name.c_str() );

    bd = MemTrackGetBlock( ptr );
    REQUIRE_NOT_NULL( bd );
    REQUIRE_EQ( name, bd->name );
    REQUIRE_NE( (clock_t)0, bd->created );
    REQUIRE_EQ( (clock_t)0, bd->freed );
    REQUIRE_EQ( (size_t)100, bd->max_size );
}

FIXTURE_TEST_CASE ( KMemTrack_Digest, MemTrackSuiteFixture )
{
    const void * ptr = (const void*)1;
    MemTrackAlloc( ptr, 100 );
    const string name = "name";
    MemTrackName( ptr, name.c_str() );
    MemTrackFree( ptr );
    MemTrackDigest( cout );
}
//////////////////////////////////////////////////// Main
extern "C" {

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion(void) { return 0x1000000; }
rc_t CC UsageSummary(const char* progname) { return 0; }

rc_t CC Usage(const Args* args) { return 0; }

const char UsageDefaultName[] = "test-mem-track";

rc_t CC KMain(int argc, char* argv[])
{
    rc_t rc = KMemTrackSuite(argc, argv);
    return rc;
}
}
