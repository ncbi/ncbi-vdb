#include <stdio.h>

#include "../prod-cmn.c"
#include "../blob-priv.h"
#include "../cursor-priv.h"

#define ASSERT_RC(FUNC, ARGS) do { rc_t const rc = FUNC ARGS; if (rc != 0) { fprintf(stderr, "%s failed: rc %u\n", #FUNC, (unsigned)rc); abort(); } } while(0)

static rc_t testRowFunc(void *const self,
                        VXformInfo const *const info,
                        int64_t const rowId,
                        VRowResult *const rslt,
                        uint32_t argc, VRowData const argv[])
{
    return 0;
}

static void rowFuncFree(void *const rf)
{
}

static void testVFunctionProdCallRowFunc(void)
{
    int self = 0;
    VCursor curs;
    VFunctionProd vfp;
    VProduction *const prod = &vfp.dad;
    VXformInfo info;
    VBlob *input = NULL;
    VBlob *rslt = NULL;
    Vector inputs;
    int64_t const start_id = 1;
    int64_t const stop_id = 1000; // UINT_MAX / 4 + 1;

    memset(&vfp, 0, sizeof(vfp));
    memset(&info, 0, sizeof(info));
    memset(&curs, 0, sizeof(curs));

#if PROD_NAME
    prod->name = "testRowFunc";
#endif
    prod->var = prodFunc;
    prod->sub = vftRow;
    prod->chain = chainDecoding;
    prod->desc.intrinsic_bits = 8;
    prod->desc.intrinsic_dim = 1;

    vfp.curs = &curs;
    vfp.fself = (void *)&self;
    vfp.whack = rowFuncFree;
    vfp.u.rf = testRowFunc;

    vfp.start_id = 1;
    vfp.stop_id = UINT32_MAX / 4 + vfp.start_id;

    VectorInit(&inputs, 0, 0);
    ASSERT_RC(VBlobNewAsArray, (&input, start_id, stop_id, 0, 8));
    ASSERT_RC(VectorSet, (&inputs, 0, input));
    ASSERT_RC(VFunctionProdCallRowFunc, (&vfp, &rslt, vfp.stop_id + 1, 1, &info, &inputs, vfp.start_id, vfp.stop_id + 1));

    VBlobRelease(input);
    VBlobRelease(rslt);
    VectorWhack(&inputs, NULL, NULL);
    VFunctionProdDestroy(&vfp);
}

int main(int argc, char *argv[])
{
    testVFunctionProdCallRowFunc();
    return 0;
}

/* this allows us to not include linker-int.c
 * which imports the transform functions
 */
rc_t VLinkerMakeIntrinsic(VLinker **lp)
{
    abort();
    return 0;
}

/* for some reason KQueue functions weren't available
 */
typedef struct KQueue KQueue;

rc_t KQueueMake(KQueue **q, uint32_t capacity)
{
    abort();
    return 0;
}

rc_t KQueueRelease(const KQueue *self)
{
    abort();
    return 0;
}

rc_t KQueuePush(KQueue *self, const void *item, struct timeout_t *tm)
{
    abort();
    return 0;
}

rc_t KQueuePop(KQueue *self, void **item, struct timeout_t *tm)
{
    abort();
    return 0;
}
