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

#include "wb-irzip-impl.h"

/* this generates many pairs of encode/decode functions for different datatypes */
#define iunzip_func_v0 test_iunzip_func_v0
#define vdb_izip test_vdb_izip
#define vdb_iunzip test_vdb_iunzip

#include "../libs/vxf/irzip.c"

#define STYPE uint64_t
rc_t doEncode_u64(uint8_t dst[], size_t dsize, size_t *used, int64_t *Min, int64_t *Slope, uint8_t *series_count,uint8_t *planes, const STYPE Y[], unsigned N)
{
    return encode_u64(dst, dsize, used, Min, Slope, series_count, planes, Y, N);
}
rc_t doDecode_u64(STYPE Y[], unsigned N, int64_t* min, int64_t* slope, uint8_t series_count, uint8_t planes, const uint8_t src[], size_t ssize)
{
    return decode_u64(Y, N, min, slope, series_count, planes, src, ssize);
}
#undef STYPE 

#define STYPE uint32_t
rc_t doEncode_u32(uint8_t dst[], size_t dsize, size_t *used, int64_t *Min, int64_t *Slope, uint8_t *series_count,uint8_t *planes, const STYPE Y[], unsigned N)
{
    return encode_u32(dst, dsize, used, Min, Slope, series_count, planes, Y, N);
}
rc_t doDecode_u32(STYPE Y[], unsigned N, int64_t* min, int64_t* slope, uint8_t series_count, uint8_t planes, const uint8_t src[], size_t ssize)
{
    return decode_u32(Y, N, min, slope, series_count, planes, src, ssize);
}
#undef STYPE 

#define STYPE int32_t
rc_t doEncode_i32(uint8_t dst[], size_t dsize, size_t *used, int64_t *Min, int64_t *Slope, uint8_t *series_count,uint8_t *planes, const STYPE Y[], unsigned N)
{
    return encode_i32(dst, dsize, used, Min, Slope, series_count, planes, Y, N);
}
rc_t doDecode_i32(STYPE Y[], unsigned N, int64_t* min, int64_t* slope, uint8_t series_count, uint8_t planes, const uint8_t src[], size_t ssize)
{
    return decode_i32(Y, N, min, slope, series_count, planes, src, ssize);
}
#undef STYPE 

#define STYPE int64_t
rc_t doEncode_i64(uint8_t dst[], size_t dsize, size_t *used, int64_t *Min, int64_t *Slope, uint8_t *series_count,uint8_t *planes, const STYPE Y[], unsigned N)
{
    return encode_i64(dst, dsize, used, Min, Slope, series_count, planes, Y, N);
}
rc_t doDecode_i64(STYPE Y[], unsigned N, int64_t* min, int64_t* slope, uint8_t series_count, uint8_t planes, const uint8_t src[], size_t ssize)
{
    return decode_i64(Y, N, min, slope, series_count, planes, src, ssize);
}
#undef STYPE 

rc_t CC iunzip_func_v0(
                      void *Self,
                      const VXformInfo *info,
                      VBlobResult *dst,
                      const VBlobData *src
                      )
{
    return RC ( rcVDB, rcFunction, rcExecuting, rcInterface, rcUnsupported ); /* should not be hit in this test */
}
