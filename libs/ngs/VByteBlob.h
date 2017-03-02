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

#ifndef _h_vbyteblob_
#define _h_vbyteblob_

#include <kfc/ctx.h>

struct VBlob;

#ifdef __cplusplus
extern "C" {
#endif

/* Calculate the biggest available contiguous data portion of the blob:
*  starts at rowId, ends at MaxRows if not 0, before a repeated value or at the end of the blob
*/
void VByteBlob_ContiguousChunk ( const struct VBlob* blob,  ctx_t ctx, int64_t rowId, uint64_t maxRows, const void** data, uint64_t* size, bool stopAtRepeat );

#ifdef __cplusplus
}
#endif

#endif /* _h_vbyteblob_ */
