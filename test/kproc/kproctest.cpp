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

#include <kapp/args.h>

#include <klib/rc.h>
#include <klib/time.h>
#include <klib/log.h>

#include <atomic.h>
#include <os-native.h>

#include <kproc/cond.h>
#include <kproc/lock.h>
#include <kproc/thread.h>
#include <kproc/timeout.h>
#include <kproc/queue.h>

#include <stdexcept>
#include <sstream>
#include <utility>
#include <atomic>

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

static KLock *makeLock() 
{
    KLock *lock = nullptr;
    if (KLockMake(&lock) != 0)
        throw logic_error("KLockMake failed");
    return lock;
}

class KLockFixture
{
public:
    KLockFixture()
    : thread(0)
    , lock(makeLock())
    , threadWaiting(false)
    {}
    
    ~KLockFixture()
    {
        if (thread != 0 && KThreadRelease(thread) != 0)
            cerr << "~KLockFixture: KThreadRelease failed" << endl;
        if (KLockRelease(const_cast<KLock const *>(lock)) != 0)
            cerr << "~KLockFixture: KLockRelease failed" << endl;
    }

protected:
    rc_t runThread() {
        LOG(LogLevel::e_message, "KLockFixture::runThread: acquiring lock, set threadWaiting to true" << endl);
        threadWaiting = true;

        while (KLockAcquire(lock) != 0)
        {
            TestEnv::SleepMs(1);
        }
        LOG(LogLevel::e_message, "KLockFixture::runThread: lock acquired" << endl);

        threadWaiting = false;
        LOG(LogLevel::e_message, "KLockFixture::runThread: set threadWaiting to false" << endl);

        auto const rc = KLockUnlock(lock);

        LOG(LogLevel::e_message, "KLockFixture::runThread: exiting" << endl);
        return rc;
    }

    static rc_t ThreadFn ( KThread const *const thread, void *const data ) noexcept {
        return reinterpret_cast<KLockFixture *>(data)->runThread();
    }
    
    rc_t startThread() {
        return KThreadMake(&thread, ThreadFn, reinterpret_cast<void *>(this));
    }

    rc_t StartThread()
    {
        threadWaiting = false;
        LOG(LogLevel::e_message, "KLockFixture::StartThread: set threadWaiting to false" << endl);

        auto const rc = startThread();
        if (rc) return rc;

        do { TestEnv::SleepMs(1); } while (!threadWaiting);
        
        LOG(LogLevel::e_message, "KLockFixture::StartThread: threadWaiting == true" << endl);
        return 0;
    }
    
    std::pair<rc_t, rc_t> waitThread() const {
        rc_t threadRc = 0, rc = KThreadWait(thread, &threadRc);
        return {rc, threadRc};
    }

public:
    KThread* thread;
    KLock *lock;
    atomic_bool threadWaiting;
};

FIXTURE_TEST_CASE(KLock_Acquire, KLockFixture)
{
    // lock
    REQUIRE_RC(KLockAcquire(lock));
    // start a thread that tries to lock, see it wait for the lock to become available
    REQUIRE_RC(StartThread());
    REQUIRE(threadWaiting);

    // unlock, see the thread finish
    REQUIRE_RC(KLockUnlock(lock));

    auto const rcs = waitThread();
    REQUIRE_RC(rcs.first);
    REQUIRE(!threadWaiting);
    REQUIRE_RC(rcs.second);
    LOG(LogLevel::e_message, "KLock_Acquire: done" << endl);
}

///////////////////////// KTimedLock

static KTimedLock *makeTimedLock() 
{
    KTimedLock *lock = nullptr;
    if (KTimedLockMake(&lock) != 0)
        throw logic_error("KLockMake failed");
    return lock;
}

TEST_CASE( KTimedLock_NULL )
{
    REQUIRE_RC_FAIL(KTimedLockMake(NULL));
}

class KTimedLockFixture
{
public:
    KTimedLockFixture()
    : thread(0)
    , lock(makeTimedLock())
    , threadWaiting(false)
    {}
    
    ~KTimedLockFixture()
    {
        if (thread != 0 && KThreadRelease(thread) != 0)
            cerr << "~KLockFixture: KThreadRelease failed" << endl;
        if (KLockRelease((KLock const *)lock) != 0)
            cerr << "~KLockFixture: KLockRelease failed" << endl;
    }

protected:
    rc_t acquireLock() const {
        auto ltm = tm; ///< make a copy
        return KTimedLockAcquire(lock, &ltm); ///< modify the local copy, no sync needed;
    }
    
    rc_t unlock() const {
        return KTimedLockUnlock(lock);
    }
    
    rc_t runThread() {
        LOG(LogLevel::e_message, "KTimedLockFixture::runThread: acquiring lock, set threadWaiting to true, timeout = " << tm.mS << "ms" << endl);
        threadWaiting = true;

        auto rc = acquireLock();
        if (rc == 0)
            LOG(LogLevel::e_message, "KTimedLockFixture::runThread: lock acquired" << endl);
        else
            LOG(LogLevel::e_message, "KTimedLockFixture::runThread: lock acquire failed" << endl);

        LOG(LogLevel::e_message, "KTimedLockFixture::runThread: set threadWaiting to false" << endl);
        threadWaiting = false;

        if (rc == 0)
            rc = unlock();
            
        LOG(LogLevel::e_message, "KTimedLockFixture::runThread: exiting" << endl);
        
        return rc;
    }
    
    static rc_t KLock_Timed_ThreadFn ( const KThread *thread, void *data ) noexcept {
        return reinterpret_cast<KTimedLockFixture *>(data)->runThread();
    }

    rc_t startThread() {
        return KThreadMake(&thread, KLock_Timed_ThreadFn, this);
    }

    rc_t StartThread(size_t timeout)
    {
        auto rc = TimeoutInit( &tm, (uint32_t)timeout );
        if (rc) return rc;
        
        threadWaiting = false;
        LOG(LogLevel::e_message, "KTimedLockFixture::StartThread: set threadWaiting to false" << endl);

        rc = startThread();
        if (rc) return rc;

        do { TestEnv::SleepMs(1); } while (!threadWaiting);
        
        LOG(LogLevel::e_message, "KTimedLockFixture::StartThread: threadWaiting == true" << endl);
        return 0;
    }
    
    std::pair<rc_t, rc_t> waitThread() const {
        rc_t threadRc = 0, rc = KThreadWait(thread, &threadRc);
        return {rc, threadRc};
    }

public:
    KThread* thread;
    KTimedLock *lock;
    atomic_bool threadWaiting;
    timeout_t tm;
};

FIXTURE_TEST_CASE(KTimedLock_Acquire, KTimedLockFixture)
{
    // lock
    REQUIRE_RC(KTimedLockAcquire(lock, NULL));

    // start a thread that tries to lock
    LOG(LogLevel::e_message, "TEST_KLock_TimedAcquire: starting thread" << endl);
    REQUIRE_RC(StartThread(1000));
    REQUIRE(threadWaiting);

    // unlock, see the thread finish
    LOG(LogLevel::e_message, "TEST_KLock_TimedAcquire: unlocking" << endl);
    REQUIRE_RC(KTimedLockUnlock(lock));

    auto const rcs = waitThread();
    REQUIRE_RC(rcs.first);
    REQUIRE(!threadWaiting);
    REQUIRE_RC(rcs.second);

    LOG(LogLevel::e_message, "TEST_KLock_TimedAcquire: done" << endl);
}

FIXTURE_TEST_CASE(KTimedLock_Acquire_Timeout, KTimedLockFixture)
{
    // lock
    REQUIRE_RC(KTimedLockAcquire(lock, NULL));

    // start a thread that tries to lock, see it time out
    REQUIRE_RC(StartThread(100));
    REQUIRE(threadWaiting);

    // do not unlock, wait for the thread to finish
    auto const rcs = waitThread();
    REQUIRE_RC(rcs.first);
    REQUIRE(!threadWaiting);

#ifdef WINDOWS
    REQUIRE_EQ(rcs.second, SILENT_RC(rcPS, rcLock, rcLocking, rcLock, rcBusy));
#else
    REQUIRE_EQ(rcs.second, SILENT_RC(rcPS, rcLock, rcLocking, rcTimeout, rcExhausted));
#endif

    REQUIRE_RC(KTimedLockUnlock(lock));
}

///////////////////////// KRWLock
TEST_CASE( KRWLock_NULL )
{
    REQUIRE_RC_FAIL(KRWLockMake(NULL));
}

static KRWLock *makeKRWLock() {
    KRWLock *lock = nullptr;
    if (KRWLockMake(&lock) != 0)
        throw logic_error("KLockFixture: KLockMake failed");
    return lock;
};

class KRWLockFixture
{
public:
    KRWLockFixture()
    : thread(0)
    , lock(makeKRWLock())
    , threadWaiting(false)
    {}
    
    ~KRWLockFixture()
    {
        if (thread != 0 && KThreadRelease(thread) != 0)
            cerr << "~KRWLockFixture: KThreadRelease failed" << endl;
        if (KRWLockRelease(const_cast<KRWLock const *>(lock)) != 0)
            cerr << "~KRWLockFixture: KLockRelease failed" << endl;
    }

protected:
    rc_t runReaderThread() {
        threadWaiting = true;

        while (KRWLockAcquireShared(lock) != 0)
        {
            TestEnv::SleepMs(1);
        }
        auto const rc = KRWLockUnlock(lock);
        threadWaiting = false;
        return rc;
    }
    static rc_t Reader_ThreadFn ( const KThread *thread, void *data )
    {
        return reinterpret_cast<KRWLockFixture *>(data)->runReaderThread();
    }
    rc_t startReaderThread() {
        return KThreadMake(&thread, Reader_ThreadFn, reinterpret_cast<void *>(this));
    }
    
    rc_t runWriterThread() {
        threadWaiting = true;

        LOG(LogLevel::e_message, "KRWLockFixture::runWriterThread: calling KRWLockAcquireExcl\n");
        auto rc = KRWLockAcquireExcl(lock);
        LOG(LogLevel::e_message, "KRWLockFixture::runWriterThread: out of KRWLockAcquireExcl\n");
        if (rc == 0)
        {
            LOG(LogLevel::e_message, "KRWLockFixture::runWriterThread: calling KRWLockUnlock\n");
            rc = KRWLockUnlock(lock);
        }
        threadWaiting = false;
        return rc;
    }
    static rc_t Writer_ThreadFn ( const KThread *thread, void *data )
    {
        return reinterpret_cast<KRWLockFixture *>(data)->runWriterThread();
    }
    rc_t startWriterThread() {
        return KThreadMake(&thread, Writer_ThreadFn, reinterpret_cast<void *>(this));
    }
    
    rc_t runReaderTimedThread() {
        threadWaiting = true;

        auto ltm = tm;
        auto rc = KRWLockTimedAcquireShared(lock, &ltm);
        if (rc == 0)
            rc = KRWLockUnlock(lock);

        threadWaiting = false;
        return rc;
    }
    static rc_t ReaderTimed_ThreadFn ( const KThread *thread, void *data )
    {
        return reinterpret_cast<KRWLockFixture *>(data)->runReaderTimedThread();
    }
    rc_t startReaderTimedThread() {
        return KThreadMake(&thread, ReaderTimed_ThreadFn, reinterpret_cast<void *>(this));
    }
    
    rc_t runWriterTimedThread() {
        threadWaiting = true;

        auto ltm = tm;
        auto rc = KRWLockTimedAcquireExcl(lock, &ltm);
        if (rc == 0)
            rc = KRWLockUnlock(lock);

        threadWaiting = false;
        return rc;
    }
    static rc_t WriterTimed_ThreadFn ( const KThread *thread, void *data ) noexcept
    {
        return reinterpret_cast<KRWLockFixture *>(data)->runWriterTimedThread();
    }
    rc_t startWriterTimedThread() {
        return KThreadMake(&thread, WriterTimed_ThreadFn, reinterpret_cast<void *>(this));
    }

    rc_t StartThread(bool writer)
    {
        auto const rc = writer ? startWriterThread() : startReaderThread();
        if (rc) return rc;
        do { TestEnv::SleepMs(1); } while (!threadWaiting);
        return 0;
    }
    
    rc_t StartThread(bool writer, size_t timeout)
    {
        auto rc = TimeoutInit( &tm, (uint32_t)timeout );
        if (rc) return rc;
        
        rc = writer ? startWriterTimedThread() : startReaderTimedThread();
        if (rc) return rc;

        do { TestEnv::SleepMs(1); } while (!threadWaiting);
        return 0;
    }

    std::pair<rc_t, rc_t> waitThread() const {
        rc_t threadRc = 0, rc = KThreadWait(thread, &threadRc);
        return {rc, threadRc};
    }

    KThread* thread;
    KRWLock *lock;
    timeout_t tm;
    atomic_bool threadWaiting;
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

    REQUIRE(threadWaiting);

    LOG(LogLevel::e_message, "KRWLock_WriterWaitsForReader: calling KRWLockUnlock\n");
    REQUIRE_RC(KRWLockUnlock(lock));
    // let the thread finish
    do { TestEnv::SleepMs(1); } while (threadWaiting);
    REQUIRE(!threadWaiting);
}

FIXTURE_TEST_CASE(KWRLock_Reader_TimedAcquire, KRWLockFixture)
{
    // lock
    REQUIRE_RC(KRWLockAcquireExcl(lock));

    // start a thread that tries to lock
    REQUIRE_RC(StartThread(false, 1000));

    // see the thread wait
    REQUIRE(threadWaiting);

    // unlock, see the thread finish
    REQUIRE_RC(KRWLockUnlock(lock));
    
    auto const rcs = waitThread();
    REQUIRE_RC(rcs.first);
    REQUIRE_RC(rcs.second);
}

FIXTURE_TEST_CASE(KWRLock_Reader_TimedAcquire_Timeout, KRWLockFixture)
{
    // lock
    REQUIRE_RC(KRWLockAcquireExcl(lock));

    // start a thread that tries to lock, see it time out
    REQUIRE_RC(StartThread(false, 500));

    // see the thread time out
    auto const rcs = waitThread();
    REQUIRE_RC(rcs.first);
    REQUIRE_EQ(rcs.second, SILENT_RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted )); // timed out

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
    REQUIRE(threadWaiting);

    // unlock, see the thread finish
    REQUIRE_RC(KRWLockUnlock(lock));

    auto const rcs = waitThread();
    REQUIRE_RC(rcs.first);
    REQUIRE_RC(rcs.second);
}

FIXTURE_TEST_CASE(KWRLock_Writer_TimedAcquire_Timeout, KRWLockFixture)
{
    // read-lock
    REQUIRE_RC(KRWLockAcquireShared(lock));

    // start a thread that tries to write-lock, see it time out
    REQUIRE_RC(StartThread(true, 500));

    // see the thread time out
    auto const rcs = waitThread();
    REQUIRE_RC(rcs.first);
    REQUIRE_EQ(rcs.second, SILENT_RC ( rcPS, rcRWLock, rcLocking, rcTimeout, rcExhausted )); // timed out

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
    :   thread(0),
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
            rc_t threadRc = 0;
            if (KThreadWait(thread, &threadRc) != 0)
                cerr << "~KConditionFixture: KThreadWait failed" << endl;
            if (threadRc != 0)
                cerr << "~KConditionFixture: thread failed, threadRc != 0" << endl;
            if (KThreadRelease(thread) != 0)
                cerr << "~KConditionFixture: KThreadRelease failed" << endl;
        }
        if (KLockRelease((const KLock*)lock) != 0)
            cerr << "~KConditionFixture: KLockRelease failed" << endl;
        if (KConditionRelease(cond) != 0)
            cerr << "~KConditionFixture: KConditionRelease failed" << endl;
    }

protected:
    rc_t runThread() {
        LOG(LogLevel::e_message, "KConditionFixture::runThread: sleeping" << endl);
        TestEnv::SleepMs(300);
        
        LOG(LogLevel::e_message, "KConditionFixture::runThread: signaling condition" << endl);
        is_signaled = true;
        
        auto const rc = do_broadcast ? KConditionBroadcast(cond) : KConditionSignal(cond);

        LOG(LogLevel::e_message, "KConditionFixture::runThread: exiting" << endl);

        return rc;
    }
    static rc_t ThreadFn ( const KThread *thread, void *data ) noexcept
    {
        return reinterpret_cast<KConditionFixture *>(data)->runThread();
    }
    rc_t startThread() {
        return KThreadMake(&thread, ThreadFn, reinterpret_cast<void *>(this));
    }

    rc_t StartThread()
    {
        LOG(LogLevel::e_message, "StartThread: starting thread" << endl);
        return startThread();
    }

public:
    KThread* thread;
    KLock* lock;
    KCondition* cond;
    atomic_bool is_signaled;
    timeout_t tm;
    bool do_broadcast;
};

FIXTURE_TEST_CASE( KCondition_TimedWait_Timeout, KConditionFixture )
{
    REQUIRE_RC(KLockAcquire(lock));
    REQUIRE_RC(TimeoutInit(&tm, 100));
    REQUIRE_RC(KConditionSignal(cond)); // signaling before waiting should not do anything
    REQUIRE_EQ(KConditionTimedWait(cond, lock, &tm), SILENT_RC ( rcPS, rcCondition, rcWaiting, rcTimeout, rcExhausted )); // timed out

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

    timeout_t tm;
    memset( & tm, 0, sizeof( tm ) );
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
            void *it = (void*)i;
            REQUIRE_RC(KQueuePush(queue, it, & tm));
        }
        REQUIRE_RC_FAIL(KQueuePush(queue, item, & tm));
        for (uint64_t i = 1; i < 3; ++i) {
            uint64_t j = 0;
            void *it = 0;
            REQUIRE_RC(KQueuePop(queue, &it, & tm));
            j = (uint64_t)it;
            REQUIRE_EQ(i, j);
        }
    }

    {   // pushed 2 = capacity (ok) - popped 3 >capacity (failure)
        for (uint64_t i = 1; i < 3; ++i) {
            void *it = (void*)i;
            REQUIRE_RC(KQueuePush(queue, it, & tm));
        }
        for (uint64_t i = 1; i < 3; ++i) {
            uint64_t j = 0;
            void *it = 0;
            REQUIRE_RC(KQueuePop(queue, &it, & tm));
            j = (uint64_t)it;
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
        if (KQueueMake(&queue, (uint32_t)nThreads) != 0)
            throw logic_error("KQueueFixture: KQueueMake failed");
    }
    ~KQueueFixture()
    {
        if (threads != NULL)
        {
            for (unsigned i = 0; i < nStartedThreads; ++i)
            {
                if (threads[i] != 0 && KThreadRelease(threads[i]) != 0)
                    cerr << "~KQueueFixture: KThreadRelease failed" << endl;
            }
        }
        free(threads);
        free(threadsData);
        free(threadRcs);

        if (KQueueRelease((const KQueue*)queue) != 0)
            cerr << "~KQueueFixture: KQueueRelease failed" << endl;
    }

protected:
    struct ThreadData {
        KQueue * queue;
        int tid;
        size_t max_tid;
        uint32_t timeout_ms;
        bool is_reader;
        atomic_bool finish; // will stop generating events and seal the queue once detected
        bool allow_timeout; // if set, we won't treat timeout as an error
    };

    class Thread {
    public:
        // danger - this should be an extern "C" function
        // with CC calling convention on Windows
        static rc_t KQueue_ThreadFn ( const KThread *thread, void *data ) noexcept
        {
            ThreadData* td = (ThreadData*)data;
            rc_t rc = 0;
            int numOps = 8192;

            for (int i = 0; i < numOps; ++i)
            {
                timeout_t tm;
                memset( & tm, 0, sizeof( tm ) );
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
        int tid = (int) (nStartedThreads++);
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
static rc_t argsHandler(int argc, char* argv[]) {
    Args* args = NULL;
    rc_t rc = ArgsMakeAndHandle(&args, argc, argv, 0, NULL, 0);
    ArgsWhack(args);
    return rc;
}

int main ( int argc, char *argv [] )
{
	// this makes messages from the test code appear
	// (same as running the executable with "-l=message")
	//TestEnv::verbosity = LogLevel::e_message;

    return KProcTestSuite( argc, argv );
}
