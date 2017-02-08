#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <klib/defs.h>
#include <klib/data-buffer.h>
#include <kdb/manager.h>
#include <kdb/column.h>

#define UNIT_TEST (1)

#define self_t encode_self_t
#define self_whack encode_self_whack
#include "../dictionary-write.c"
#undef self_t
#undef self_whack

#define self_t decode_self_t
#define self_whack decode_self_whack
#include "../dictionary-read.c"
#undef self_t
#undef self_whack

#define TEST_SIZE (1024u * 1024u)
#define VALUE_COUNT (1024u * 2u)

static char *testValue[VALUE_COUNT];

static void generateValues()
{
    char value[256];
    int i;
    
    srand(time(0));
    for (i = 0; i < VALUE_COUNT; ++i) {
        int j;
        
        for (j = 0; j < 255; ++j) {
            static char const ch64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz0123456789";
            int ch;
            ch = ch64[random() % 64];
            if (ch == 0)
                break;
            value[j] = ch;
        }
        value[j] = 0;
        testValue[i] = strdup(value);
    }
}

static void test(KDBManager *mgr)
{
    KColumn *kcol;
    rc_t rc = KDBManagerCreateColumn(mgr, &kcol, kcmInit, kcmMD5, 0, "FOO");
    VFuncDesc desc;
    KDataBuffer data;
    VRowResult result;
    
    VFuncDesc wdesc;
    KDataBuffer wdata;
    VRowResult wresult;

    assert(rc == 0);

    KDataBufferMake(&data, 8, 1);
    memset(&result, 0, sizeof(result));
    result.data = &data;

    KDataBufferMake(&wdata, 32, 1);
    memset(&wresult, 0, sizeof(wresult));
    wresult.data = &wdata;
    
    rc = make_dict_text_insert(&wdesc, kcol);
    assert(rc == 0);
    rc = make_dict_text_lookup(&desc, kcol, wdesc.self, getValueFromCurrent);
    assert(rc == 0);
    {
        int i;
        for (i = 0; i < TEST_SIZE; ++i) {
            VRowData wrow;
            VRowData row;
            uint32_t const original = random() % VALUE_COUNT;
            
            memset(&wrow, 0, sizeof(wrow));
            wrow.u.data.elem_bits = 8;
            wrow.u.data.base = testValue[original];
            wrow.u.data.elem_count = strlen(wrow.u.data.base);
            
            rc = wdesc.u.ndf(wdesc.self, NULL, i + 1, &wresult, 1, &wrow);
            if (rc) break;
            
            memset(&row, 0, sizeof(row));
            row.u.data.elem_bits = 32;
            row.u.data.base = wresult.data->base;
            row.u.data.elem_count = 1;

            rc = desc.u.ndf(desc.self, NULL, i + 1, &result, 1, &row);
            if (rc) break;
            
            if (memcmp(testValue[original], data.base, data.elem_count) != 0) {
                fprintf(stderr, "expected: %s\ngot: %.*s\n", testValue[original], (int)data.elem_count, data.base);
                abort();
            }
        }
    }
    KColumnRelease(kcol);
}

int main(int argc, char *argv[])
{
    KDBManager *mgr;
    rc_t rc;
    
    generateValues();
    rc = KDBManagerMakeUpdate(&mgr, NULL);
    if (rc == 0) {
        test(mgr);
        KDBManagerRelease(mgr);
    }
    else {
        fprintf(stderr, "failed to create manager\n");
    }
    return 0;
}
