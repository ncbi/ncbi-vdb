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

#ifndef _h_kfc_ctx_
#define _h_kfc_ctx_

#ifndef _h_kfc_extern_
#include <kfc/extern.h>
#endif

#ifndef _h_kfc_defs_
#include <kfc/defs.h>
#endif

#ifndef _h_kfc_rc_
#include <kfc/rc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KRsrc;


/*--------------------------------------------------------------------------
 * KSourceLoc
 *  holds a static block identifying module, file and file extension.
 *  by including this file in a source compilation, a static block
 *  called "s_src_loc" will be automatically defined.
 */
typedef struct KSourceLoc KSourceLoc;
struct KSourceLoc
{
    const char * mod;
    const char * file;
    const char * ext;
};

#ifndef SRC_LOC_DEFINED
static KSourceLoc s_src_loc = { __mod__, __file__, __fext__ };
#define SRC_LOC_DEFINED 1
#endif


/*--------------------------------------------------------------------------
 * KFuncLoc
 *  holds a static block identifying function and source file.
 */
typedef struct KFuncLoc KFuncLoc;
struct KFuncLoc
{
    const KSourceLoc * src;
    const char * func;
    rc_t rc_context;
};

#define DECLARE_FUNC_LOC( rc_mod, rc_targ, rc_ctx )                \
    static KFuncLoc s_func_loc =                                   \
    { & s_src_loc, __func__, RAW_CTX ( rc_mod, rc_targ, rc_ctx ) }


/*--------------------------------------------------------------------------
 * ctx_t
 *  modified from vdb-3
 */
typedef struct KCtx KCtx;
struct KCtx
{
    struct KRsrc const * rsrc;
    const KFuncLoc * loc;
    const KCtx * caller;

    /* record call-stack depth */
    uint32_t zdepth;

    /* here for hybrid-mode */
    volatile rc_t rc;

    /* event */
    void * volatile evt;
};

/* assert
 * rsrc_assert
 */
#define ctx_assert( ctx )                       \
    assert ( ctx != NULL )
#define ctx_rsrc_assert( ctx )                  \
    ctx_assert ( ctx );                         \
    assert ( ctx -> rsrc != NULL )


/* init
 *  initialize local context block
 *  performs a particular contortion to allow:
 *   a) initialization of a local ctx_t block
 *   b) reassignment of "ctx" pointer to new block
 *   c) use as an initializer to allow declarations to follow
 */
static __inline__
KCtx ctx_init ( KCtx * new_ctx, ctx_t * ctxp, const KFuncLoc * func_loc )
{
    /* extract caller's ctx pointer */
    ctx_t ctx = * ctxp;

    /* construct a local copy of ctx_t as return value
       allow compiler to zero trailing members more
       efficiently than by explicit initialization */
    KCtx local_ctx = { ctx -> rsrc, func_loc, ctx, ctx -> zdepth + 1 };

    /* reassign the "ctx" pointer for stack frame */
    * ctxp = new_ctx;

    /* "return" the new block - a noop that permits
       use of this function as an initializer, and
       further declarations of local variables */
    return local_ctx;
}


/* FUNC_ENTRY
 *  establishes context within a function
 *  legal to use wherever a normal declaration is allowed
 */
#define FUNC_ENTRY( ctx, rc_mod, rc_targ, rc_ctx )                      \
    DECLARE_FUNC_LOC ( rc_mod, rc_targ, rc_ctx );                       \
    KCtx local_ctx = ctx_init ( & local_ctx, & ( ctx ), & s_func_loc )


/* POP_CTX [ RESERVED USAGE ]
 *  unlinks ctx for a stack frame
 *  used ONLY for code that wants to remove itself from call chain
 *  after having executed FUNC_ENTRY, i.e. for thunk operations
 *
 *  normally, a thunk or other piece of code that does not want
 *  to participate in the call chain ctx linkage would simply
 *  avoid using FUNC_ENTRY. however, this can be dangerous if any
 *  errors occur during operation.
 *
 *  to handle this condition, a thunk-like function can use
 *  FUNC_ENTRY in a normal fashion, but unlink with POP_CTX
 *  just prior to calling through to the next function, usually
 *  as a tail-call.
 */
#define POP_CTX( ctx ) \
    ctx = ctx -> caller


/* recover
 *  queries thread for previously stored KRsrc block
 *  creates a new one if necessary
 */
KFC_EXTERN ctx_t CC ctx_recover ( KCtx * new_ctx, const KFuncLoc * func_loc );


/* HYBRID_FUNC_ENTRY
 *  recovers or creates thread-specific KRsrc block
 *  initializes a local ctx_t
 *  declares a local ctx pointer variable
 */
#define HYBRID_FUNC_ENTRY( rc_mod, rc_targ, rc_ctx )            \
    KCtx local_ctx;                                             \
    DECLARE_FUNC_LOC ( rc_mod, rc_targ, rc_ctx );               \
    ctx_t ctx = ctx_recover ( & local_ctx, & s_func_loc )

#ifdef __cplusplus
}
#endif

#endif /* _h_kfc_ctx_ */
