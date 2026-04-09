#include <stdio.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

#define _DEBUGGING 1
#include <klib/debug.h>
#include <klib/rc.h>
#include <klib/writer.h>
#include <align/align-access.h>
#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>
#include <kdb/manager.h>
#include <kdb/kdb-priv.h>
#include <vfs/manager.h>
#include <vfs/path.h>
#include <iostream>

using namespace std;



#define USE_SDK_LOCK


/*
#ifdef _MSC_VER
#include <io.h>
#include <minwinbase.h>
CRITICAL_SECTION sdk_mutex;
void InitMutex() {
    InitializeCriticalSection(&sdk_mutex);
}
#define OUTLock() EnterCriticalSection(&sdk_mutex)
#define OUTUnlock() LeaveCriticalSection(&sdk_mutex)
#  ifdef USE_SDK_LOCK
#    define SDKLock() EnterCriticalSection(&sdk_mutex)
#    define SDKUnlock() LeaveCriticalSection(&sdk_mutex)
#  else
#    define SDKLock() do{}while(0)
#    define SDKUnlock() do{}while(0)
#  endif
#else
# include <unistd.h>
# define SDKLock() do{}while(0)
# define SDKUnlock() do{}while(0)
# define OUTLock() do{}while(0)
# define OUTUnlock() do{}while(0)
#endif
*/


static mutex sdk_mutex;
static bool use_mutex = false;

void OUTLock() {
    sdk_mutex.lock();
}

void OUTUnlock() {
    sdk_mutex.unlock();
}

void SDKLock() {
    if (use_mutex) sdk_mutex.lock();
}

void SDKUnlock() {
    if (use_mutex) sdk_mutex.unlock();
}


void CheckRc(rc_t rc, const char* code, const char* file, int line)
{
    if (rc) {
        char buffer1[4096];
        size_t error_len;
        RCExplain(rc, buffer1, sizeof(buffer1), &error_len);
        char buffer2[8192];
        int len = snprintf(buffer2, sizeof(buffer2), "%s:%d: %s failed: %#x: %s",
            file, line, code, rc, buffer1);
        if (int(sizeof(buffer2)) < len) len = int(sizeof(buffer2));
        string msg(buffer2, len);
        SDKLock();
        cerr << "RC: " << msg << endl;
        SDKUnlock();
        exit(1);
    }
}


#define RC_CALL(call) \
    do { \
        SDKLock(); \
        rc_t rc = (call); \
        SDKUnlock(); \
        CheckRc(rc, #call, __FILE__, __LINE__); \
    } while (0)

int LowLevelTest(int idx, const string& acc)
{
    OUTLock();
    cout << "thread " << idx << " id " << acc << endl;
    OUTUnlock();
    const VDBManager* mgr = 0;
    RC_CALL(VDBManagerMakeRead(&mgr, 0));

    const VDatabase* db = 0;
    RC_CALL(VDBManagerOpenDBRead(mgr, &db, 0, acc.c_str()));

    const VTable* seq_table = 0;
    RC_CALL(VDatabaseOpenTableRead(db, &seq_table, "SEQUENCE"));

    const VCursor* seq_cursor = 0;
    RC_CALL(VTableCreateCursorRead(seq_table, &seq_cursor));
    RC_CALL(VCursorPermitPostOpenAdd(seq_cursor));
    RC_CALL(VCursorOpen(seq_cursor));

    uint32_t SPOT_GROUP;
    RC_CALL(VCursorAddColumn(seq_cursor, &SPOT_GROUP, "SPOT_GROUP"));
    uint32_t READ_TYPE;
    RC_CALL(VCursorAddColumn(seq_cursor, &READ_TYPE, "READ_TYPE"));
    uint32_t READ_LEN;
    RC_CALL(VCursorAddColumn(seq_cursor, &READ_LEN, "READ_LEN"));
    uint32_t READ_START;
    RC_CALL(VCursorAddColumn(seq_cursor, &READ_START, "READ_START"));
    uint32_t READ;
    RC_CALL(VCursorAddColumn(seq_cursor, &READ, "READ"));
    uint32_t QUALITY;
    RC_CALL(VCursorAddColumn(seq_cursor, &QUALITY, "QUALITY"));
    uint32_t TRIM_LEN;
    RC_CALL(VCursorAddColumn(seq_cursor, &TRIM_LEN, "TRIM_LEN"));
    uint32_t TRIM_START;
    RC_CALL(VCursorAddColumn(seq_cursor, &TRIM_START, "TRIM_START"));
    uint32_t NAME;
    RC_CALL(VCursorAddColumn(seq_cursor, &NAME, "NAME"));

    int64_t seq_row = 38012;
    {
        const void* data;
        uint32_t bit_offset, bit_length;
        uint32_t elem_count;
        for (int i = 0; i < 2; ++i) {
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                READ_TYPE,
                &bit_length, &data, &bit_offset,
                &elem_count));
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                READ_TYPE,
                &bit_length, &data, &bit_offset,
                &elem_count));
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                READ_LEN,
                &bit_length, &data, &bit_offset,
                &elem_count));
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                READ_START,
                &bit_length, &data, &bit_offset,
                &elem_count));
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                TRIM_START,
                &bit_length, &data, &bit_offset,
                &elem_count));
            RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
                TRIM_LEN,
                &bit_length, &data, &bit_offset,
                &elem_count));
        }

        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            NAME,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            NAME,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            READ_LEN,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            READ_START,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            TRIM_START,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            TRIM_LEN,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            READ,
            &bit_length, &data, &bit_offset,
            &elem_count));

        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            READ_TYPE,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            READ_LEN,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            READ_START,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            TRIM_START,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            TRIM_LEN,
            &bit_length, &data, &bit_offset,
            &elem_count));

        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            NAME,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            NAME,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            READ_LEN,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            READ_START,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            TRIM_START,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            TRIM_LEN,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row,
            READ,
            &bit_length, &data, &bit_offset,
            &elem_count));

        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row + 1,
            READ_TYPE,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row + 1,
            READ_TYPE,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row + 1,
            READ_LEN,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row + 1,
            READ_START,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row + 1,
            READ_START,
            &bit_length, &data, &bit_offset,
            &elem_count));
        RC_CALL(VCursorCellDataDirect(seq_cursor, seq_row + 1,
            TRIM_LEN,
            &bit_length, &data, &bit_offset,
            &elem_count));
    }
    RC_CALL(VCursorRelease(seq_cursor));
    RC_CALL(VTableRelease(seq_table));
    RC_CALL(VDatabaseRelease(db));
    RC_CALL(VDBManagerRelease(mgr));
    return 0;
}


static const vector<string> accessions = {
    "SRR389414"
    // "SRR1551783", "SRR389414", "SRR494733", "SRR505887"
};

static int n_repeats = 10;

void LowLevelAllTests(int idx)
{
    OUTLock();
    cout << "thread " << idx << " started" << endl;
    OUTUnlock();
    for (size_t i = 0; i < accessions.size() * n_repeats; ++i) {
        if (LowLevelTest(idx, accessions[rand() % accessions.size()]) != 0) return;
    }
    OUTLock();
    cout << "thread " << idx << " finished" << endl;
    OUTUnlock();
}


int main(int argc, const char* argv[])
{
    int n_threads = 30;
    if (argc > 1) {
        use_mutex = atoi(argv[1]) != 0;
    }
    if (argc > 2) {
        n_threads = atoi(argv[2]);
    }
    if (argc > 3) {
        n_repeats = atoi(argv[3]);
    }

    // use_mutex = false;
    // int n_threads = 2;
    // n_repeats = 2;

    //KWrtInit("ApplicationName", 0);
    //KDbgSetString("KNS");

    vector<thread> threads;
    for (int i = 0; i < n_threads; ++i) {
        threads.push_back(thread(LowLevelAllTests, i));
    }
    for (auto& t : threads) t.join();

    return 0;
}
