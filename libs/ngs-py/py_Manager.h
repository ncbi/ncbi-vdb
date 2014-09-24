#ifndef _Included_py_ngs_ncbi_Manager
#define _Included_py_ngs_ncbi_Manager
#ifdef __cplusplus
extern "C" {
#endif

#include "py_ngs_defs.h"
#include <stddef.h>

PY_RES_TYPE PY_NGS_Engine_ReadCollectionMake(char const* spec, void** ppReadCollection, char* pStrError, size_t nStrErrorBufferSize);
/*
These functions are not needed:
*ReadCollection can be released with Release functon from ngs-sdk
*We don't export yet another string from engine

PY_RES_TYPE PY_NGS_Engine_RefcountRelease(void* pRefcount, void** ppNGSStrError);
PY_RES_TYPE PY_NGS_Engine_StringData(void const* pNGSString, char const** pRetBufPtr);
PY_RES_TYPE PY_NGS_Engine_StringSize(void const* pNGSString, size_t* pRetSize);
*/

#ifdef __cplusplus
}
#endif
#endif
