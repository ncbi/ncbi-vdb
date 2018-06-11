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

#ifndef _h_bytecode_
#define _h_bytecode_

#include <klib/rc.h>

struct VProduction;
struct VBlobMRUCacheCursorContext;
struct VXformInfo;
struct Vector;
struct VBlob;

typedef struct ByteCodeContext ByteCodeContext;
struct ByteCodeContext
{
    struct VProduction * self;

    int64_t id;
    uint32_t cnt;
    struct VBlobMRUCacheCursorContext * cctx;

    /* functions */
    const struct VXformInfo *info;
    struct Vector *args;
    int64_t param_start_id;
    int64_t param_stop_id;

    uint32_t elem_bits;
    const struct VBlob * sblob;

    struct VBlob * result;
    rc_t    rc;
};

enum ByteCodeOperation
{
    bcProductionReadBlob,
    bcPre_ProductionReadBlob,
    bcPost_ProductionReadBlob,
    bcSimpleProdRead,
    bcPost_SimpleProdRead,
    bcFunctionProdRead,
    bcFunctionProdSelect,
    bcFunctionProdReadNormal,
    bcPre_FunctionProdReadNormal,
    bcPost_FunctionProdReadNormal,
    bcScriptProdRead,
    bcPhysicalProdRead,
    bcColumnProdRead,
    bcSimpleProdPage2Blob,
    bcSimpleProdSerial2Blob,
    bcSimpleProdBlob2Serial,
    bcFunctionProdCallCompare,
    bcFunctionProdCallCompare1,
    bcFunctionProdCallLegacyBlobFunc,
    bcFunctionProdCallNDRowFunc,
    bcFunctionProdCallRowFunc,
    bcFunctionProdCallArrayFunc,
    bcFunctionProdCallPageFunc,
    bcFunctionProdCallBlobFunc,
    bcPre_FunctionProdCallBlobFunc,
    bcPost_FunctionProdCallBlobFunc,
    bcFunctionProdCallBlobNFunc,
    bcFunctionProdCallByteswap,
    bcPhysicalRead,
    bcWPhysicalRead, /* bcPhysicalRead for write side */
    bcPhysicalReadKColumn,
    bcWColumnReadBlob,
    bcFunctionProdCallBlobFuncEncoding,
    bcFunctionProdCallBlobFuncDecoding,
    bcPhysicalReadBlob,
    bcPhysicalWrite,
    bcPre_PhysicalWrite,
    bcPost_PhysicalWrite,
    bcPhysicalConvertStatic,
    bcPre_PhysicalConvertStatic,
    bcPost_PhysicalConvertStatic,
    bcPhysicalWriteKColumn,
    bcPhysicalReadStatic,
    bcReturn,            /* end of sequence, return to caller */
    bcEnd
};

rc_t ExecuteByteCode ( enum ByteCodeOperation, struct VProduction * prod, ByteCodeContext * ctx );

typedef rc_t ( * ByteCodeOperation_fn ) ( struct VProduction * prod, ByteCodeContext * ctx );

typedef ByteCodeOperation_fn ByteCodeTable_t [];

extern ByteCodeTable_t ByteCode_Default;

#endif /* _h_prod_priv_ */
