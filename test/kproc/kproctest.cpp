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
* Unit tests for KProc interfaces
*/

#include <ktst/unit_test.hpp>

#include <klib/rc.h>

#include <atomic32.h>
#include <os-native.h>

#include <kproc/cond.h>
#include <kproc/lock.h>
#include <kproc/thread.h>
#include <kproc/timeout.h>

#include <stdexcept>

using namespace std;
using namespace ncbi::NK;

TEST_SUITE( KProcTestSuite );

//TODO: KThread

///////////////////////// KLock
TEST_CASE( KLock_NULL )
{
    REQUIRE_RC_FAIL(KLockMake(NULL));
}

class KLockFixture
{
public:
    KLockFixture()
    :   threadRc(0),
        thread(0),
        lock(0)
    {
        if (KLockMake(&lock) != 0)
            throw logic_error("KLockFixture: KLockMake failed");
    }
    ~KLockFixture()
    {
        if (thread != 0 && KThreadRelease(thread) != 0)
            throw logic_error("~KLockFixture: KThreadRelease failed");
        if (KLockRelease((const KLock*)lock) != 0)
            throw logic_error("~KLockFixture: KLockRelease failed");
    }
    
protected:
    class Thread {
    public:
        // danger - this should be an extern "C" function
        // with CC calling convention on Windows
        static rc_t KLock_ThreadFn ( const KThread *thread, void *data )
        {
            KLockFixture* self = (KLockFixture*)data;
            
            LOG(LogLevel::e_message, "KLock_ThreadFn acquiring lock, set threadWaiting to 1" << endl);    
            atomic32_set ( & self->threadWaiting, 1 );
            
            while (KLockAcquire(self->lock) != 0)
            {
                TestEnv::SleepMs(1);
            }
            LOG(LogLevel::e_message, "KLock_ThreadFn: lock acquired" << endl);    
            
            atomic32_set ( & self->threadWaiting, 0 );
            LOG(LogLevel::e_message, "KLock_ThreadFn: set threadWaiting to 0" << endl);    
            
            self->threadRc = KLockUnlock(self->lock);
            LOG(LogLevel::e_message, "KLock_Timed_ThreadFn: exiting" << endl);    
            return 0;
        }
    };
    
    rc_t StartThread()
    {
        atomic32_set ( & threadWaiting, 0 );
        LOG(LogLevel::e_message, "StartThread: set threadWaiting to 0" << endl);    

        threadRc = 0;
        rc_t rc = KThreadMake(&thread, Thread::KLock_ThreadFn, this);
        while (threadRc == 0 && !atomic32_read (&threadWaiting))
        {
            TestEnv::SleepMs(1);
        }
        LOG(LogLevel::e_message, "StartThread: threadWaiting == 1" << endl);    
        return rc;
    }
    
public:
    rc_t threadRc;
    KThread* thread;
    volatile atomic32_t threadWaiting;
    timeout_t tm;
    KLock* lock;
};

FIXTURE_TEST_CASE(KLock_Acquire, KLockFixture)
{
    // lock 
    REQUIRE_RC(KLockAcquire(lock));
    // start a thread that tries to lock, see it wait for the lock to become available
    REQUIRE_RC(StartThread()); // makes sure threadWaiting == 1
    
    // unlock, see the thread finish
    REQUIRE_RC(KLockUnlock(lock));
    while (atomic32_read (&threadWaiting))
    {
        TestEnv::SleepMs(1);
    }

    REQUIRE_RC(threadRc);
    LOG(LogLevel::e_message, "KLock_Acquire: done" << endl);    
}

///////////////////////// KTimedLock
TEST_CASE( KTimedLock_NULL )
{
    REQUIRE_RC_FAIL(KTimedLockMake(NULL));
}

class KTimedLockFixture
{
public:
    KTimedLockFixture()
    :   threadRc(0),
        thread(0),
        lock(0)
    {
        if (KTimedLockMake(&lock) != 0)
            throw logic_error("KLockFixture: KLockMake failed");
    }
    ~KTimedLockFixture()
    {
        if (thread != 0 && KThreadRelease(thread) != 0)
            throw logic_error("~KLockFixture: KThreadRelease failed");
        if (KLockRelease((const KLock*)lock) != 0)
            throw logic_error("~KLockFixture: KLockRelease failed");
    }
    
protected:
    class Thread {
    public:
        // danger - this should be an extern "C" function
        // with CC calling convention on Windows
        static rc_t KLock_Timed_ThreadFn ( const KThread *thread, void *data )
        {
            KTimedLockFixture* self = (KTimedLockFixture*)data;
            
            LOG(LogLevel::e_message, "KLock_Timed_ThreadFn acquiring lock, set threadWaiting to 1, timeout = " << self->tm.mS << "ms" << endl);    
            atomic32_set ( & self->threadWaiting, 1 );
        
            self->threadRc = KTimedLockAcquire(self->lock, &self->tm);
            if (self->threadRc == 0)
                LOG(LogLevel::e_message, "KLock_Timed_ThreadFn: lock acquired" << endl);    
            else
                LOG(LogLevel::e_message, "KLock_Timed_ThreadFn: lock acquire failed" << endl);    
            
            LOG(LogLevel::e_message, "KLock_Timed_ThreadFn: set threadWaiting to 0" << endl);    
            
            if (self->threadRc == 0)
                self->threadRc = KTimedLockUnlock(self->lock);
            LOG(LogLevel::e_message, "KLock_Timed_ThreadFn: exiting" << endl);    
            atomic32_set ( & self->threadWaiting, 0 );
            return 0;
        }
    };
    
    rc_t StartThread(size_t timeout)
    {
        rc_t rc = TimeoutInit( &tm, timeout );
        if ( rc == 0)
        {
            atomic32_set ( & threadWaiting, 0 );
            LOG(LogLevel::e_message, "StartTimedThread: set threadWaiting to 0" << endl);    
            
            threadRc = 0;
            rc = KThreadMake(&thread, Thread::KLock_Timed_ThreadFn, this);
            while (threadRc == 0 && !atomic32_read (&threadWaiting))
            {
                TestEnv::SleepMs(1);
            }
            LOG(LogLevel::e_message, "StartTimedThread: threadWaiting == 1" << endl);    
        }
        return rc;
    }

public:    
    rc_t threadRc;
    KThread* thread;
    volatile atomic32_t threadWaiting;
    timeout_t tm;
    KTimedLock* lock;
};

FIXTURE_TEST_CASE(KTimedLock_Acquire, KTimedLockFixture)
{
    // lock 
    REQUIRE_RC(KTimedLockAcquire(lock, NULL));
    
    // start a thread that tries to lock
    LOG(LogLevel::e_message, "TEST_KLock_TimedAcquire: starting thread" << endl);    
    REQUIRE_RC(StartThread(1000));// makes sure threadWaiting == 1

    // unlock, see the thread finish
    LOG(LogLevel::e_message, "TEST_KLock_TimedAcquire: unlocking" << endl);    
    REQUIRE_RC(KTimedLockUnlock(lock));
    
    // wait for the thread to finish
    while (atomic32_read (&threadWaiting))
    {
        TestEnv::SleepMs(1);
    }
    REQUIRE_RC(threadRc);

    LOG(LogLevel::e_message, "TEST_KLock_TimedAcquire: done" << endl);    
}
FIXTURE_TEST_CASE(KTimedLock_Acquire_Timeout, KTimedLockFixture)
{
    // lock 
    REQUIRE_RC(KTimedLockAcquire(lock, NULL));
    
    // start a thread that tries to lock, see it time out
    REQUIRE_RC(StartThread(100));// makes sure threadWaiting == 1
    
    // do not unlock, wait for the thread to finish
    while (atomic32_read (&threadWaiting))
    {
        TestEnv::SleepMs(1);
    }
    REQUIRE_EQ(threadRc, RC ( rcPS, rcLock, rcLocking, rcTimeout, rcExhausted )); // timed out
    
    REQUIRE_RC(KTimedLockUnlock(lock));
}

///////////////////////// KRWLock
TEST_CASE( KRWLock_NULL )
{
    REQUIRE_RC_FAIL(KRWLockMake(NULL));
}

class KRWLockFixture
{
public:
    KRWLockFixture()
    :   threadRc(0),
        thread(0),
        lock(0)
    {
        atomic32_set ( & threadWaiting, 0 );
        
        if (KRWLockMake(&lock) != 0)
            throw logic_error("KLockFixture: KLockMake failed");
    }
    ~KRWLockFixture()
    {
        if (thread != 0 && KThreadRelease(thread) != 0)
            throw logic_error("~KRWLockFixture: KThreadRelease failed");
        if (KRWLockRelease((const KRWLock*)lock) != 0)
            throw logic_error("~KRWLockFixture: KLockRelease failed");
    }
    
protected:
    class Thread {
    public:
        static rc_t KRWLock_Reader_ThreadFn ( const KThread *thread, void *data )
        {
            KRWLockFixture* self = (KRWLockFixture*)data;
            atomic32_set( & self->threadWaiting, true );
            
            while (KRWLockAcquireShared(self->lock) != 0)
            {
                TestEnv::SleepMs(1);
            }
            self->threadRc = KRWLockUnlock(self->lock);
            atomic32_set( & self->threadWaiting, false );
            return 0;
        }
        static rc_t KRWLock_Writer_ThreadFn ( const KThread *thread, void *data )
        {
            KRWLockFixture* self = (KRWLockFixture*)data;
            atomic32_set( & self->threadWaiting, true );
            
            LOG(LogLevel::e_message, "KRWLock_Writer_ThreadFn: calling KRWLockAcquireExcl\n");                
            self->threadRc = KRWLockAcquireExcl(self->lock);
            LOG(LogLevel::e_message, "KRWLock_Writer_ThreadFn: out of KRWLockAcquireExcl\n");                
            if (self->threadRc == 0)
            {
                LOG(LogLevel::e_message, "KRWLock_Writer_ThreadFn: calling KRWLockUnlock\n");                
                self->threadRc = KRWLockUnlock(self->lock);
            }
            atomic32_set( & self->threadWaiting, false );
            return 0;
        }
        static rc_t KRWLock_ReaderTimed_ThreadFn ( const KThread *thread, void *data )
        {
            KRWLockFixture* self = (KRWLockFixture*)data;
            atomic32_set( & self->threadWaiting, true );
            
            self->threadRc = KRWLockTimedAcquireShared(self->lock, &self->tm);
            if (self->threadRc == 0)
                self->threadRc = KRWLockUnlock(self->lock);
            atomic32_set( & self->threadWaiting, false );
            return 0;
        }
        static rc_t KRWLock_WriterTimed_ThreadFn ( const KThread *thread, void *data )
        {
            KRWLockFixture* self = (KRWLockFixture*)data;
            atomic32_set( & self->threadWaiting, true );
            
            self->threadRc = KRWLockTimedAcquireExcl(self->lock, &self->tm);
            if (self->threadRc == 0)
                self->threadRc = KRWLockUnlock(self->lock);
            atomic32_set( & self->threadWaiting, false );
            return 0;
        }
    };
    
    rc_t StartThread(bool writer)
    {
        threadRc = 0;
        rc_t rc = KThreadMake(&thread, writer ? Thread::KRWLock_Writer_ThreadFn : Thread::KRWLock_Reader_ThreadFn, this);
        while (!atomic32_read (&threadWaiting))
        {
            TestEnv::SleepMs(1);
        }
        return rc;
    }
    rc_t StartThread(bool writer, size_t timeout)
    {
        rc_t rc = TimeoutInit( &tm, timeout );
        if ( rc == 0)
        {
            threadRc = 0;
            rc = KThreadMake(&thread, writer ? Thread::KRWLock_WriterTimed_ThreadFn : Thread::KRWLock_ReaderTimed_ThreadFn, this);
            while (!atomic32_read (&threadWaiting))
            {
                TestEnv::SleepMs(1);
            }
        }
        return rc;
    }

    volatile atomic32_t threadWaiting;
    rc_t threadRc;
    KThread* thread;
    timeout_t tm;
    KRWLock* lock;
};

FIXTURE_TEST_CASE( KRWLock_ManyReaders, KRWLockFixture )
{
    // get multiple read locks
    REQUIRE_RC(KRWLockAcquireShared(lock));
    REQUIRE_RC(KRWLockAcquireShared(lock));
    REQUIRE_RC(KRWLockAcquireShared(lock));
    //NB: On Linux, KRWLock goes away without a leak even if Unlock is not called.
    // On Windows, Unlocks are required.
    KRWLockUnlock(lock);
    KRWLockUnlock(lock);
    KRWLockUnlock(lock);
}

FIXTURE_TEST_CASE( KRWLock_OneWriter, KRWLockFixture )
{
    REQUIRE_RC(KRWLockAcquireExcl(lock));
    //NB: trying to lock again from the same thread errors out on Linux but hangs on Windows
    //REQUIRE_RC_FAIL(KRWLockAcquireExcl(lock));
    //REQUIRE_RC_FAIL(KRWLockAcquireShared(lock));
    //TODO: try to acquire from a different thread
    
    REQUIRE_RC(KRWLockUnlock(lock));

    // now, can lock again
    REQUIRE_RC(KRWLockAcquireShared(lock));
    REQUIRE_RC(KRWLockUnlock(lock));
}

FIXTURE_TEST_CASE( KRWLock_WriterWaitsForReader, KRWLockFixture )
{
    LOG(LogLevel::e_message, "KRWLock_WriterWaitsForReader: calling KRWLockAcquireShared\n");                
    REQUIRE_RC(KRWLockAcquireShared(lock));
    
    // start a thread that tries to write-lock, see it wait
    LOG(LogLevel::e_message, "KRWLock_WriterWaitsForReader: starting thread\n");                
    REQUIRE_RC(StartThread(true));
    
    REQUIRE(atomic32_read ( & threadWaiting )); 

    LOG(LogLevel::e_message, "KRWLock_WriterWaitsForReader: calling KRWLockUnlock\n");                
    REQUIRE_RC(KRWLockUnlock(lock));
    // let the thread finish
    while (atomic32_read (&threadWaiting))
    {
        TestEnv::SleepMs(1);
    }
    REQUIRE(!atomic32_read ( & threadWaiting )); 
}

FIXTURE_TEST_CASE(KWRLock_Reader_TimedAcquire, KRWLockFixture)
{
    // lock 
    REQUIRE_RC(KRWLockAcquireExcl(lock));
    
    // start a thread that tries to lock
    REQUIRE_RC(StartThread(false, 1000));
    
    // see the thread wait
    REQUIRE(atomic32_read ( & threadWaiting )); 
    
    // unlock, see the thread finish
    REQUIRE_RC(KRWLockUnlock(lock));
    while (atomic32_read (&threadWaiting))
    {
        TestEnv::SleepMs(1);
    }
    REQUIRE_RC(threadRc);
}

FIXTURE_TEST_CASE(KWRLock_Reader_TimedAcquire_Timeout, KRWLockFixture)
{
    // lock 
    REQUIRE_RC(KRWLockAcquireExcl(lock));
    
    // start a thread that tries to lock, see it time out
    REQUIRE_RC(StartThread(false, 500));

    // see the thread time out
    while (atomic32_read (&threadWaiting))
    {
        TestEnv::SleepMs(1);
    }
    rc_t req_rc = RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted );
    REQUIRE_EQ(threadRc, req_rc); // timed out
    
    REQUIRE_RC(KRWLockUnlock(lock));
}

FIXTURE_TEST_CASE(KWRLock_Writer_TimedAcquire, KRWLockFixture)
{
    // read-lock 
    REQUIRE_RC(KRWLockAcquireShared(lock));
    
    // start a thread that tries to write-lock and see it wait
    REQUIRE_RC(StartThread(true, 1000));
    
    // see the thread wait
    TestEnv::SleepMs(300);
    REQUIRE(atomic32_read (&threadWaiting));
    
    // unlock, see the thread finish
    REQUIRE_RC(KRWLockUnlock(lock));
    while (atomic32_read (&threadWaiting))
    {
        TestEnv::SleepMs(1);
    }
    REQUIRE_RC(threadRc);
}

FIXTURE_TEST_CASE(KWRLock_Writer_TimedAcquire_Timeout, KRWLockFixture)
{
    // read-lock 
    REQUIRE_RC(KRWLockAcquireShared(lock));
    
    // start a thread that tries to write-lock, see it time out
    REQUIRE_RC(StartThread(true, 500));
    
    // see the thread time out
    while (atomic32_read (&threadWaiting))
    {
        TestEnv::SleepMs(1);
    }
    REQUIRE_EQ(threadRc, RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted )); // timed out
    
    REQUIRE_RC(KRWLockUnlock(lock));
}

//KCondition
TEST_CASE( KCondition_NULL )
{
    REQUIRE_RC_FAIL(KConditionMake(NULL));
}
TEST_CASE( KCondition_MakeRelease )
{
    KCondition* cond;
    REQUIRE_RC(KConditionMake(&cond));
    REQUIRE_RC(KConditionRelease(cond));
}

//TODO: KConditionWait, KConditionTimedWait, KConditionSignal, KConditionBroadcast

//TODO: KSemaphore
//TODO: KQueue
//TODO: Timeout
//TODO: KBarrier (is it used anywhere? is there a Windows implementation?)

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>

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


const char UsageDefaultName[] = "test-kproc";

rc_t CC KMain ( int argc, char *argv [] )
{
	// this makes messages from the test code appear
	// (same as running the executable with "-l=message")
	//TestEnv::verbosity = LogLevel::e_message;
    
    rc_t rc = KProcTestSuite( argc, argv );
    return rc;
}

}
