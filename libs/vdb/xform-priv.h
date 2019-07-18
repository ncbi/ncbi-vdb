/*=======================================================================================
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

#ifndef _h_xform_priv_
#define _h_xform_priv_

#ifndef _h_vdb_xform_
#include <vdb/xform.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* VBlobFuncN
 *  works on VBlob objects
 */
struct VBlob;

enum
{
    vftBlobN = vftBlob + 1,
    vftSelect,
    vftPassThrough,

    vftLastFuncProto
};

/* merge and other internal functions
 *
 * If you return one of your input blobs, ***** ADDREF ***** it.
 */
typedef rc_t ( CC * VBlobFuncN ) ( void *self, const VXformInfo *info, int64_t row_id,
    struct VBlob **rslt, uint32_t argc, struct VBlob const *argv [] );

typedef rc_t ( CC * VBlobCompareFunc ) (void *self, const VRowData *input, const VRowData *read);

typedef union VFuncDescInternalFuncs VFuncDescInternalFuncs;
union VFuncDescInternalFuncs
{
    VRowFunc         rf;
    VNonDetRowFunc   ndf;
    VFixedRowFunc    pf;
    VArrayFunc       af;
    VBlobFunc        bf;
    VBlobFuncN       bfN;
    VBlobCompareFunc cf;
};

#define VFUNCDESC_INTERNAL_FUNCS( DESC ) \
    ( ( union VFuncDescInternalFuncs * ) ( & ( DESC ) -> u ) )

/* factory declaration with no self parameter */
#define VTRANSFACT_BUILTIN_IMPL( fact, maj, min, rel ) \
    VTRANSFACT_IMPL ( fact, maj, min, rel )

#ifdef __cplusplus
}
#endif

#endif /* _h_xform_priv_ */
