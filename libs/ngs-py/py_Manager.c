#include "py_Manager.h"

#include <kfc/ctx.h>
#include <kfc/rsrc.h>
#include <kfc/except.h>
#include <kfc/xc.h>
#include <kfc/rc.h>

#include <kfc/rsrc-global.h>

#include "NGS_String.h"
#include "NGS_ReadCollection.h"
#include "NGS_ReferenceSequence.h"
#include "NGS_Refcount.h"

#include <assert.h>
#include <string.h>

static PY_RES_TYPE NGSErrorHandler(ctx_t ctx, char* pStrError, size_t nStrErrorBufferSize)
{
    char const* pszErrorDesc = WHAT();
    assert(pStrError);
    strncpy(pStrError, pszErrorDesc, nStrErrorBufferSize);
    pStrError[nStrErrorBufferSize - 1] = '\n';
    CLEAR();
    return PY_RES_ERROR; /* TODO: return error (exception) type */
}


LIB_EXPORT PY_RES_TYPE PY_NGS_Engine_ReadCollectionMake(char const* spec, void** ppReadCollection, char* pStrError, size_t nStrErrorBufferSize)
{
    HYBRID_FUNC_ENTRY(rcSRA, rcMgr, rcConstructing);

    void* pRet = (void*)NGS_ReadCollectionMake(ctx, spec);

    if (FAILED())
    {
        return NGSErrorHandler(ctx, pStrError, nStrErrorBufferSize);
    }

    assert(pRet != NULL);
    assert(ppReadCollection != NULL);

    *ppReadCollection = pRet;

    CLEAR();
    return PY_RES_OK;
}

LIB_EXPORT PY_RES_TYPE PY_NGS_Engine_ReferenceSequenceMake(char const* spec, void** ppReadCollection, char* pStrError, size_t nStrErrorBufferSize)
{
    HYBRID_FUNC_ENTRY(rcSRA, rcMgr, rcConstructing);

    void* pRet = (void*)NGS_ReferenceSequenceMake(ctx, spec);

    if (FAILED())
    {
        return NGSErrorHandler(ctx, pStrError, nStrErrorBufferSize);
    }

    assert(pRet != NULL);
    assert(ppReadCollection != NULL);

    *ppReadCollection = pRet;

    CLEAR();
    return PY_RES_OK;
}

#if 0
PY_RES_TYPE PY_NGS_Engine_RefcountRelease(void* pRefcount, void** ppNGSStrError)
{
    HYBRID_FUNC_ENTRY(rcSRA, rcRefcount, rcReleasing);

    NGS_RefcountRelease((NGS_Refcount*)pRefcount, ctx);

    /* TODO: use TRY-CATCH or ON_FAIL macros*/
    if (FAILED() /*ctx->rc*/)
    {
        return NGSErrorHandler(ctx, ppNGSStrError);
    }

    CLEAR();
    return PY_RES_OK;
}

PY_RES_TYPE PY_NGS_Engine_StringData(void const* pNGSString, char const** pRetBufPtr/* TODO: add new error return? */)
{
    HYBRID_FUNC_ENTRY ( rcSRA, rcString, rcAccessing );
    assert(pRetBufPtr);
    *pRetBufPtr = NGS_StringData(pNGSString, ctx);
    return PY_RES_OK;
}

PY_RES_TYPE PY_NGS_Engine_StringSize(void const* pNGSString, size_t* pRetSize/*TODO: add new error return?*/)
{
    HYBRID_FUNC_ENTRY ( rcSRA, rcString, rcAccessing );
    assert(pRetSize);
    *pRetSize = NGS_StringSize(pNGSString, ctx);
    return PY_RES_OK;
}
#endif



