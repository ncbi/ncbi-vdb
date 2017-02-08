#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

static uint32_t encoded[TEST_SIZE];
static uint32_t original[TEST_SIZE];
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

static void testWrite(KDBManager *mgr)
{
    KColumn *kcol;
    rc_t rc = KDBManagerCreateColumn(mgr, &kcol, kcmInit, kcmMD5, 0, "FOO");
    int i;
    
    if (rc == 0) {
        VFuncDesc desc;
        
        rc = make_dict_text_insert(&desc, kcol);
        if (rc == 0) {
            encode_self_t const *const encoder = desc.self;
            KDataBuffer data;
            VRowResult result;
            
            KDataBufferMake(&data, 32, 1);
            memset(&result, 0, sizeof(result));
            result.data = &data;
            
            for (i = 0; i < TEST_SIZE; ++i) {
                VRowData row;

                memset(&row, 0, sizeof(row));
                row.u.data.elem_bits = 8;
                row.u.data.base = testValue[original[i] = random() % VALUE_COUNT];
                row.u.data.elem_count = strlen(row.u.data.base);
                
                rc = desc.u.ndf(desc.self, NULL, i + 1, &result, 1, &row);
                if (rc) break;
                
                encoded[i] = *(uint32_t *)data.base;
            }
            KDataBufferWhack(&data);
            desc.whack(desc.self);

            printf("dictionary blobs: %u\n", encoder->blobCount);
            printf("total compressed blob size: %zu\n", encoder->totalCompressedBlobSize);
            printf("total keys carried over: %u\n", encoder->keysCarried);
            printf("compression ratio: %lf\n", ((double)encoder->totalValueSize)/encoder->totalCompressedBlobSize);
        }
        KColumnRelease(kcol);
    }
    else {
        fprintf(stderr, "failed to open column - %i\n", rc);
    }
}

static void testRead(KDBManager const *mgr)
{
    KColumn const *kcol;
    rc_t rc = KDBManagerOpenColumnRead(mgr, &kcol, "FOO");
    if (rc == 0) {
        VFuncDesc desc;
        
        rc = make_dict_text_lookup(&desc, kcol, NULL, NULL);
        if (rc == 0) {
            KDataBuffer data;
            VRowResult result;
            VRowData row;
            uint32_t key = 0;
            int i;
            
            KDataBufferMake(&data, 8, 0);
            memset(&result, 0, sizeof(result));
            result.data = &data;
            
            memset(&row, 0, sizeof(row));
            row.u.data.elem_bits = 32;
            row.u.data.base = &key;
            row.u.data.elem_count = 1;
            
            for (i = 0; i < TEST_SIZE; ++i) {
                char const *const expected = testValue[original[i]];
                
                key = encoded[i];
                rc = desc.u.ndf(desc.self, NULL, i + 1, &result, 1, &row);
                if (memcmp(expected, data.base, data.elem_count) != 0) {
                    fprintf(stderr, "expected: %s\ngot: %.*s\n", expected, (int)data.elem_count, data.base);
                    abort();
                }
            }
            KDataBufferWhack(&data);
        }
        fprintf(stderr, "success!\n");
        desc.whack(desc.self);
        KColumnRelease(kcol);
    }
    else {
        fprintf(stderr, "failed to open column - %i\n", rc);
    }
}

int main(int argc, char *argv[])
{
    KDBManager *mgr;
    rc_t rc;
    
    generateValues();
    rc = KDBManagerMakeUpdate(&mgr, NULL);
    if (rc == 0) {
        testWrite(mgr);
        testRead(mgr);
        KDBManagerRelease(mgr);
    }
    else {
        fprintf(stderr, "failed to create manager\n");
    }
    return 0;
}
