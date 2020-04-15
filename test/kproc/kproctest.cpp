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
#include <klib/time.h>
#include <klib/log.h>

#include <atomic32.h>
#include <os-native.h>

#include <kproc/cond.h>
#include <kproc/lock.h>
#include <kproc/thread.h>
#include <kproc/timeout.h>
#include <kproc/queue.h>

#include <stdexcept>
#include <sstream>

#include <cstring> // mamset

using namespace std;
using namespace ncbi::NK;

extern "C" { static rc_t argsHandler(int argc, char* argv[]); }
TEST_SUITE_WITH_ARGS_HANDLER(KProcTestSuite, argsHandler);

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

#ifdef WINDOWS
FIXTURE_TEST_CASE(KTimedLock_Acquire_Busy, KTimedLockFixture)
{
    // lock 
    REQUIRE_RC(KTimedLockAcquire(lock, NULL));
    
    // start a thread that tries to lock, see it error out
    REQUIRE_RC(StartThread(100));// makes sure threadWaiting == 1
    
    // do not unlock, wait for the thread to finish
    while (atomic32_read (&threadWaiting))
    {
        TestEnv::SleepMs(1);
    }
    REQUIRE_EQ(threadRc, RC(rcPS, rcLock, rcLocking, rcLock, rcBusy)); 
    
    REQUIRE_RC(KTimedLockUnlock(lock));
}
#else
FIXTURE_TEST_CASE(KTimedLock_Acquire_Timeout, KTimedLockFixture)
{
    // lock 
    REQUIRE_RC(KTimedLockAcquire(lock, NULL));

    // start a thread that tries to lock, see it time out
    REQUIRE_RC(StartThread(100));// makes sure threadWaiting == 1

    // do not unlock, wait for the thread to finish
    while (atomic32_read(&threadWaiting))
    {
        TestEnv::SleepMs(1);
    }
    REQUIRE_EQ(threadRc, RC(rcPS, rcLock, rcLocking, rcTimeout, rcExhausted)); // timed out

    REQUIRE_RC(KTimedLockUnlock(lock));
}
#endif

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

class KConditionFixture
{
public:
    KConditionFixture()
    :   threadRc(0),
        thread(0),
        lock(0),
        is_signaled(false),
        do_broadcast(false)
    {
        if (KLockMake(&lock) != 0)
            throw logic_error("KConditionFixture: KLockMake failed");
        if (KConditionMake(&cond) != 0)
            throw logic_error("KConditionFixture: KConditionMake failed");
    }
    ~KConditionFixture()
    {
        if (thread != 0)
        {
            if (KThreadWait(thread, NULL) != 0)
                throw logic_error("~KConditionFixture: KThreadWait failed");
            if (threadRc != 0)
                throw logic_error("~KConditionFixture: thread failed, threadRc != 0");
            if (KThreadRelease(thread) != 0)
                throw logic_error("~KConditionFixture: KThreadRelease failed");
        }
        if (KLockRelease((const KLock*)lock) != 0)
            throw logic_error("~KConditionFixture: KLockRelease failed");
        if (KConditionRelease(cond) != 0)
            throw logic_error("~KConditionFixture: KConditionRelease failed");
    }

protected:
    class Thread {
    public:
        // danger - this should be an extern "C" function
        // with CC calling convention on Windows
        static rc_t KCondition_ThreadFn ( const KThread *thread, void *data )
        {
            KConditionFixture* self = (KConditionFixture*)data;

            LOG(LogLevel::e_message, "KCondition_ThreadFn: sleeping" << endl);
            TestEnv::SleepMs(300);
            LOG(LogLevel::e_message, "KCondition_ThreadFn: signaling condition" << endl);
            self->is_signaled = true;
            if (!self->do_broadcast)
                self->threadRc = KConditionSignal(self->cond);
            else
                self->threadRc = KConditionBroadcast(self->cond);

            LOG(LogLevel::e_message, "KCondition_ThreadFn: exiting" << endl);
            return 0;
        }
    };

    rc_t StartThread()
    {
        LOG(LogLevel::e_message, "StartThread: starting thread" << endl);

        threadRc = 0;
        rc_t rc = KThreadMake(&thread, Thread::KCondition_ThreadFn, this);
        return rc;
    }

public:
    rc_t threadRc;
    KThread* thread;
    timeout_t tm;
    KLock* lock;
    KCondition* cond;
    bool is_signaled;
    bool do_broadcast;
};

FIXTURE_TEST_CASE( KCondition_TimedWait_Timeout, KConditionFixture )
{
    REQUIRE_RC(KLockAcquire(lock));
    REQUIRE_RC(TimeoutInit(&tm, 100));
    REQUIRE_RC(KConditionSignal(cond)); // signaling before waiting should not do anything
    REQUIRE_EQ(KConditionTimedWait(cond, lock, &tm), RC ( rcPS, rcCondition, rcWaiting, rcTimeout, rcExhausted )); // timed out

    REQUIRE_RC(KLockUnlock(lock));
}

FIXTURE_TEST_CASE( KCondition_TimedWait_Signaled, KConditionFixture )
{
    is_signaled = false;

    REQUIRE_RC(KLockAcquire(lock));

    REQUIRE_RC(StartThread());
    REQUIRE_RC(KConditionWait(cond, lock));
    REQUIRE(is_signaled == true);

    REQUIRE_RC(KLockUnlock(lock));
}

FIXTURE_TEST_CASE( KCondition_TimedWait_Signaled_Broadcast, KConditionFixture )
{
    is_signaled = false;
    do_broadcast = true;

    REQUIRE_RC(KLockAcquire(lock));

    REQUIRE_RC(StartThread());
    REQUIRE_RC(KConditionWait(cond, lock));
    REQUIRE(is_signaled == true);

    REQUIRE_RC(KLockUnlock(lock));
}

///////////////////////// KQueue
TEST_CASE( KQueue_NULL )
{
    REQUIRE_RC_FAIL(KQueueMake(NULL, 1));
}

TEST_CASE(KQueueSimpleTest) {
    KQueue * queue = NULL;
    REQUIRE_RC(KQueueMake(&queue, 2));

    timeout_t tm = { 0 };
    void *item = NULL;
    {   // pushed 2 - popped 2 = ok
        for (uint64_t i = 1; i < 3; ++i) {
            item = (void*)i;
            REQUIRE_RC(KQueuePush(queue, item, & tm));
        }
        for (uint64_t i = 1; i < 3; ++i) {
            uint64_t j = 0;
            REQUIRE_RC(KQueuePop(queue, &item, & tm));
            j = (uint64_t)item;
            REQUIRE_EQ(i, j);
        }
    }

    {   // pushed 3 > capacity (failure) - popped 2 (ok)
        for (uint64_t i = 1; i < 3; ++i) {
            void *item = (void*)i;
            REQUIRE_RC(KQueuePush(queue, item, & tm));
        }
        REQUIRE_RC_FAIL(KQueuePush(queue, item, & tm));
        for (uint64_t i = 1; i < 3; ++i) {
            uint64_t j = 0;
            void *item = 0;
            REQUIRE_RC(KQueuePop(queue, &item, & tm));
            j = (uint64_t)item;
            REQUIRE_EQ(i, j);
        }
    }

    {   // pushed 2 = capacity (ok) - popped 3 >capacity (failure)
        for (uint64_t i = 1; i < 3; ++i) {
            void *item = (void*)i;
            REQUIRE_RC(KQueuePush(queue, item, & tm));
        }
        for (uint64_t i = 1; i < 3; ++i) {
            uint64_t j = 0;
            void *item = 0;
            REQUIRE_RC(KQueuePop(queue, &item, & tm));
            j = (uint64_t)item;
            REQUIRE_EQ(i, j);
        }
        REQUIRE_RC_FAIL(KQueuePop(queue, &item, & tm));
    }

    REQUIRE_RC(KQueueRelease(queue));
}

class KQueueFixture
{
public:
    KQueueFixture()
    :   threadRcs(NULL),
        threads(NULL),
        threadsData(NULL),
        nThreads(32),
        nStartedThreads(0),
        sealed(false)
    {
        threads = (KThread**)malloc(sizeof(*threads) * nThreads);
        if (threads == NULL)
            throw logic_error("KQueueFixture: threads malloc failed");
        threadsData = (ThreadData*)calloc(nThreads, sizeof(*threadsData));
        if (threadsData == NULL)
            throw logic_error("KQueueFixture: threadsData malloc failed");
        threadRcs = (rc_t*)calloc(nThreads, sizeof(*threadRcs));
        if (threadRcs == NULL)
            throw logic_error("KQueueFixture: threadRcs calloc failed");
        if (KQueueMake(&queue, nThreads) != 0)
            throw logic_error("KQueueFixture: KQueueMake failed");
    }
    ~KQueueFixture()
    {
        if (threads != NULL)
        {
            for (unsigned i = 0; i < nStartedThreads; ++i)
            {
                if (threads[i] != 0 && KThreadRelease(threads[i]) != 0)
                    throw logic_error("~KQueueFixture: KThreadRelease failed");
            }
        }
        free(threads);
        free(threadsData);
        free(threadRcs);

        if (KQueueRelease((const KQueue*)queue) != 0)
            throw logic_error("~KQueueFixture: KQueueRelease failed");
    }

protected:
    struct ThreadData {
        KQueue * queue;
        int tid;
        size_t max_tid;
        uint32_t timeout_ms;
        bool is_reader;
        bool finish; // will stop generating events and seal the queue once detected
        bool allow_timeout; // if set, we won't treat timeout as an error
    };

    class Thread {
    public:
        // danger - this should be an extern "C" function
        // with CC calling convention on Windows
        static rc_t KQueue_ThreadFn ( const KThread *thread, void *data )
        {
            ThreadData* td = (ThreadData*)data;
            rc_t rc = 0;
            int numOps = 8192;

            for (int i = 0; i < numOps; ++i)
            {
                timeout_t tm = { 0 };
                timeout_t* tm_p = &tm;
                void * item;
                if (tm_p != NULL)
                    rc = TimeoutInit(tm_p, td->timeout_ms);
                if (rc != 0)
                {
                    LOG(LogLevel::e_fatal_error, "KQueue_ThreadFn: TimeoutInit failed\n");
                    break;
                }
                if (td->finish)
                {
                    rc = KQueueSeal(td->queue);
                    if (rc != 0)
                        LOG(LogLevel::e_fatal_error, "KQueue_ThreadFn: failed to seal queue\n");
                    break;
                }
                else if (td->is_reader)
                {
                    rc = KQueuePop(td->queue, &item, tm_p);
                    if (rc == 0 && (item == NULL || (uint64_t)item > td->max_tid))
                    {
                        std::stringstream ss;
                        ss << "KQueue_ThreadFn: KQueuePop returned invalid item: " << (uint64_t)item << "\n";
                        LOG(LogLevel::e_fatal_error, ss.str().c_str());
                        rc = RC(rcExe, rcQueue, rcValidating, rcItem, rcOutofrange);
                        break;
                    }
                }
                else
                {
                    item = reinterpret_cast<void*>(td->tid);
                    rc = KQueuePush(td->queue, item, tm_p);
                }

                if (rc != 0)
                {
                    if (td->allow_timeout && GetRCObject ( rc ) == (enum RCObject)rcTimeout)
                        rc = 0;
                    else if (GetRCObject ( rc ) == (enum RCObject)rcData && GetRCState ( rc ) == rcDone)
                        break;
                    else if (GetRCObject ( rc ) == (enum RCObject)rcQueue && GetRCState ( rc ) == rcReadonly)
                        break;
                    else
                    {
                        LOGERR ( klogFatal, rc, "KQueue_ThreadFn: failed to push/pop to/from queue" );
                        break;
                    }
                }

            }

            return rc;
        }
    };

    void StartThread(bool is_reader, bool allow_timeout, uint32_t timeout_ms)
    {
        if (nStartedThreads >= nThreads)
            throw logic_error("StartThread: too many threads requested");
        if (sealed)
            throw logic_error("StartThread: cannot start new thread, fixture is already sealed");

        rc_t rc;
        int tid = nStartedThreads++;
        ThreadData* td;

        td = &threadsData[tid];
        td->tid = tid;
        td->max_tid = nThreads - 1;
        td->is_reader = is_reader;
        td->allow_timeout = allow_timeout;
        td->queue = queue;
        td->timeout_ms = timeout_ms;
        rc = KThreadMake(&threads[tid], Thread::KQueue_ThreadFn, td);
        if (rc != 0)
            throw logic_error("StartThread: KThreadMake failed");
    }

    void StartThreads(int numReaders, int numWriters, bool allow_timeout = true, uint32_t timeout_ms = 0)
    {
        if (numReaders + numWriters + nStartedThreads > nThreads)
            throw logic_error("RunThreads: too many threads requested");
        if (numReaders <= 0)
            throw logic_error("RunThreads: please specify at least one reader");
        if (numWriters <= 0)
            throw logic_error("RunThreads: please specify at least one writer");

        for (int i = 0; i < numReaders; ++i)
        {
            StartThread(true, allow_timeout, timeout_ms);
        }
        for (int i = 0; i < numWriters; ++i)
        {
            StartThread(false, allow_timeout, timeout_ms);
        }
    }

    void WaitThreads(bool checkRcs = true)
    {
        rc_t rc = 0;
        sealed = true;
        for (unsigned i = 0; i < nStartedThreads; ++i)
        {
            rc_t rc2 = KThreadWait(threads[i], &threadRcs[i]);
            if (rc2 != 0)
            {
                LOG(LogLevel::e_fatal_error, "KThreadWait: KThreadWait failed\n");
                if (rc == 0)
                    rc = rc2;
            }
        }
        if (rc != 0)
            throw logic_error("WaitThreads: KThreadWait failed");
        if (checkRcs)
            CheckThreadsRc();

    }
    void CheckThreadsRc()
    {
        for (unsigned i = 0; i < nStartedThreads; ++i)
        {
            if (threadRcs[i] != 0)
                throw logic_error("CheckThreadsRc: thread returned unexpected exit code");
        }
    }

public:
    rc_t* threadRcs;
    KThread** threads;
    ThreadData* threadsData;
    size_t nThreads;
    size_t nStartedThreads;
    KQueue* queue;
    bool sealed;
};

FIXTURE_TEST_CASE(KQueue_Single_Reader_Single_Writer, KQueueFixture)
{
    StartThreads(1, 1);
    WaitThreads();
}

FIXTURE_TEST_CASE(KQueue_Multi_Reader_Single_Writer, KQueueFixture)
{
    StartThreads(31, 1);
    WaitThreads();
}

FIXTURE_TEST_CASE(KQueue_Single_Reader_Multi_Writer, KQueueFixture)
{
    StartThreads(1, 31);
    WaitThreads();
}

FIXTURE_TEST_CASE(KQueue_Multi_Reader_Multi_Writer, KQueueFixture)
{
    StartThreads(16, 16, false, 5000);
    WaitThreads();
}

FIXTURE_TEST_CASE(KQueue_Multi_Reader_Single_Writer_Seal, KQueueFixture)
{
    KTimeMs_t timeBefore = KTimeMsStamp();
    const int numReaders = 31;
    const int timeoutMs = 5000;
    StartThreads(numReaders, 1, false, timeoutMs);
    threadsData[numReaders].finish = true;
    WaitThreads(false);
    KTimeMs_t timeAfter = KTimeMsStamp();
    for (unsigned i = 0; i < nStartedThreads; ++i)
    {
        rc_t expectedRc = (i == numReaders) ? 0 : SILENT_RC ( rcCont, rcQueue, rcRemoving, rcData, rcDone );
        REQUIRE_EQ ( threadRcs[i], expectedRc );
    }
    REQUIRE_LT ( (int)(timeAfter - timeBefore), timeoutMs );
}

FIXTURE_TEST_CASE(KQueue_Single_Reader_Multi_Writer_Seal, KQueueFixture)
{
    KTimeMs_t timeBefore = KTimeMsStamp();
    const int numWriters = 31;
    const int timeoutMs = 5000;
    StartThreads(1, numWriters, false, timeoutMs);
    threadsData[0].finish = true;
    WaitThreads(false);
    KTimeMs_t timeAfter = KTimeMsStamp();
    for (unsigned i = 0; i < nStartedThreads; ++i)
    {
        rc_t expectedRc = (i == 0) ? 0 : SILENT_RC ( rcCont, rcQueue, rcInserting, rcQueue, rcReadonly );
        REQUIRE_EQ ( threadRcs[i], expectedRc );
    }
    REQUIRE_LT ( (int)(timeAfter - timeBefore), timeoutMs );
}

//TODO: KConditionWait, KConditionTimedWait, KConditionSignal, KConditionBroadcast

//TODO: KSemaphore
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

static rc_t argsHandler(int argc, char* argv[]) {
    Args* args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

rc_t CC KMain ( int argc, char *argv [] )
{
	// this makes messages from the test code appear
	// (same as running the executable with "-l=message")
	//TestEnv::verbosity = LogLevel::e_message;
    
    rc_t rc = KProcTestSuite( argc, argv );
    return rc;
}

}
