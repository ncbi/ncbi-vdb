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
* Unit tests for cacheteefile3
*/

#include <cstring>
#include <cstdlib>

#include <ktst/unit_test.hpp>

#include <klib/out.h>
#include <klib/rc.h>
#include <klib/time.h>

#include <kproc/thread.h>
#include <kproc/queue.h>
#include <kproc/lock.h>
#include <kproc/timeout.h>
#include <kproc/cond.h>

using namespace std;

TEST_SUITE( SimpleQTests );

const int num_values = 32;
const uint32_t timeout_milliseconds = 1000;

class TOFixture
{
    public:
        timeout_t to;
    
        TOFixture ()
        {
            timeout_init ( timeout_milliseconds );
        }

        void timeout_init ( uint32_t msec )
        {
            TimeoutInit ( &to, msec );
        }
};

class SQFixture : public TOFixture
{
    public:
        KQueue * q;    
        int values[ num_values ];
        
        SQFixture () : TOFixture(), q( NULL )
        {
            for ( int i = 0; i < num_values; ++i ) values[ i ] = i + 1;
        }
        
        ~SQFixture () {}
        
        rc_t push_some( KQueue * q, int count, timeout_t * tm )
        {
            rc_t rc = 0;
            for ( int i = 0; rc == 0 && i < count; ++i )
                rc = KQueuePush ( q, &( values[ i ] ), tm );
            return rc;
        }
};

FIXTURE_TEST_CASE( SimpleQ_1, SQFixture )
{
    // CASE 1 : make and release
    REQUIRE_RC( KQueueMake ( &q, num_values ) );
    REQUIRE_RC( KQueueRelease ( q ) );
}

FIXTURE_TEST_CASE( SimpleQ_2, SQFixture )
{
    // CASE 2 : make, fill up, and release
    REQUIRE_RC( KQueueMake ( &q, num_values ) );
    REQUIRE_RC( push_some( q, num_values, NULL ) );
    REQUIRE_RC( KQueueRelease ( q ) );
}

FIXTURE_TEST_CASE( SimpleQ_3, SQFixture )
{
    // CASE 3 : make, fill up, read back, and release
    REQUIRE_RC( KQueueMake ( &q, num_values ) );
    REQUIRE_RC( push_some( q, num_values, NULL ) );

    for ( int i = 0; i < num_values; ++i )
    {
        int * value;
        REQUIRE_RC( KQueuePop ( q, ( void ** )&value, NULL ) );
        REQUIRE_EQ( * value, values[ i ] );
    }
    
    REQUIRE_RC( KQueueRelease ( q ) );
}

FIXTURE_TEST_CASE( SimpleQ_4, SQFixture )
{
    // CASE 4 : with timeout - make, fill up, read back, and release
    REQUIRE_RC( KQueueMake ( &q, num_values ) );
    REQUIRE_RC( push_some( q, num_values, &to ) );

    for ( int i = 0; i < num_values; ++i )
    {
        int * value;
        REQUIRE_RC( KQueuePop ( q, ( void ** )&value, &to ) );
        REQUIRE_EQ( * value, values[ i ] );
    }
    
    REQUIRE_RC( KQueueRelease ( q ) );
}

FIXTURE_TEST_CASE( SimpleQ_5, SQFixture )
{
    // CASE 5 : with timeout - make, fill up, push one more, and release
    REQUIRE_RC( KQueueMake ( &q, num_values ) );
    REQUIRE_RC( push_some( q, num_values, &to ) );

    REQUIRE_RC_FAIL( KQueuePush ( q, &( values[ 0 ] ), &to ) );
    REQUIRE_EQ( ( uint32_t )0, TimeoutRemaining ( &to ) );
    
    REQUIRE_RC( KQueueRelease ( q ) );
}

FIXTURE_TEST_CASE( SimpleQ_6, SQFixture )
{
    // CASE 6 : with timeout - make, fill up, read back, read one more, and release
    REQUIRE_RC( KQueueMake ( &q, num_values ) );
    REQUIRE_RC( push_some( q, num_values, &to ) );

    int * value;    
    for ( int i = 0; i < num_values; ++i )
    {
        REQUIRE_RC( KQueuePop ( q, ( void ** )&value, &to ) );
        REQUIRE_EQ( * value, values[ i ] );
    }
    
    REQUIRE_RC_FAIL( KQueuePop ( q, ( void ** )&value, &to ) );
    REQUIRE_EQ( ( uint32_t )0, TimeoutRemaining ( &to ) );
    
    REQUIRE_RC( KQueueRelease ( q ) );
}

class CONDFixture : public TOFixture
{
    public:
        KCondition * cond;
        
        CONDFixture () : TOFixture(), cond( NULL )
        {
        }
};

FIXTURE_TEST_CASE( SimpleCOND_1, CONDFixture )
{
    REQUIRE_RC( KConditionMake ( &cond ) );
    REQUIRE_RC( KConditionRelease ( cond ) );
}

static rc_t CC thread_func_2( const KThread *self, void *data )
{
    KSleep( 1 );
    KCondition * cond = ( KCondition * )data;
    return KConditionSignal ( cond );
}

FIXTURE_TEST_CASE( SimpleCOND_2, CONDFixture )
{
    REQUIRE_RC( KConditionMake ( &cond ) );
    
    KLock * lock;
    REQUIRE_RC( KLockMake ( &lock ) );
    
    KThread * t;
    REQUIRE_RC( KThreadMake ( &t, thread_func_2, cond ) );
    KOutMsg( "COND2: Thread started\n" );

    REQUIRE_RC( KLockAcquire ( lock ) );    
    REQUIRE_RC( KConditionWait ( cond, lock ) );
    KOutMsg( "COND2: condition received\n" );    
    REQUIRE_RC( KLockUnlock ( lock ) );

    rc_t rc_thread;
    REQUIRE_RC( KThreadWait ( t, &rc_thread ) );
    REQUIRE_RC( rc_thread );
    REQUIRE_RC( KThreadRelease ( t ) );
    
    REQUIRE_RC( KLockRelease ( lock ) );    
    REQUIRE_RC( KConditionRelease ( cond ) );
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

rc_t CC UsageSummary ( const char * progname )
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "simple_q";

#define OPTION_DUMMY    "dummy"
#define ALIAS_DUMMY     "d"

static const char * dummy_usage[] = { "dummy argument", NULL };

OptDef TestOptions[] =
{
    { OPTION_DUMMY, ALIAS_DUMMY, NULL, dummy_usage, 1, false,  false }
};

rc_t CC KMain ( int argc, char *argv [] )
{
    Args * args;
    rc_t rc = ArgsMakeAndHandle( &args, argc, argv,
            1, TestOptions, sizeof TestOptions / sizeof TestOptions [ 0 ] );
    if ( rc == 0 )
    {
        rc = SimpleQTests( argc, argv );
    }
    return rc;
}

}
