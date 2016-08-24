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

#include <kfc/extern.h>
#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>
#include <kfc/rc.h>

#include <kfc/refcount-impl.h>

#include <string.h>
#include <sysalloc.h>


/*--------------------------------------------------------------------------
 * KRefcount
 */

KITFTOK_DEF ( KRefcount_v1 );

/* Duplicate
 *  create a duplicate reference
 *
 *  "rm" [ IN ] - capabilities to remove ( default 0 )
 */
LIB_EXPORT void * CC KRefcountDuplicate_v1 ( KRefcount_v1 * self, ctx_t ctx, caps_t rm )
{
    FUNC_ENTRY ( ctx, rcRuntime, rcRefcount, rcCopying );

    int add_result;
    const KRefcount_v1_vt * vt;

    if ( self == NULL )
        return NULL;

    vt = KVTABLE_CAST ( self -> vt, ctx, KRefcount_v1 );
    if ( vt == NULL )
        INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KRefcount_v1 interface" );
    else
    {
        if ( vt -> dup != NULL )
            return ( * vt -> dup ) ( self, ctx, rm );

        if ( rm != 0 )
            INTERNAL_ERROR ( xcParamInvalid, "this reference does not support removal of capabilities ( %b )", rm );
        else
        {
            add_result = KRefcountAdd ( & self -> refcount, self -> vt -> name );
            switch ( add_result )
            {
            case krefOkay:
                return self;
            case krefZero:
                INTERNAL_ERROR ( xcSelfZombie, "zero refcount" );
                break;
            case krefLimit:
                INTERNAL_ERROR ( xcRefcountOutOfBounds, "maximum references reached" );
                break;
            case krefNegative:
                INTERNAL_ERROR ( xcSelfZombie, "negative refcount" );
                break;
            default:
                INTERNAL_ERROR ( xcUnexpected, "unknown result from KRefcountAdd: ( %d )", add_result );
            }
        }
    }

    return NULL;
}

/* Release
 *  release a reference
 */
LIB_EXPORT void CC KRefcountRelease_v1 ( KRefcount_v1 * self, ctx_t ctx )
{
    if ( self != NULL )
    {
        FUNC_ENTRY ( ctx, rcRuntime, rcRefcount, rcReleasing );

        const KRefcount_v1_vt * vt = KVTABLE_CAST ( self -> vt, ctx, KRefcount_v1 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "this object does not support the KRefcount_v1 interface" );
        else
        {
            int drop_result = KRefcountDrop ( & self -> refcount, self -> vt -> name );
            switch ( drop_result )
            {
            case krefOkay:
                return;
            case krefWhack:
                if ( vt -> destroy != NULL )
                    ( * vt -> destroy ) ( self, ctx );
                free ( self );
                return;
            case krefZero:
                INTERNAL_WARNING ( xcSelfZombie, "zero refcount" );
                break;
            case krefNegative:
                INTERNAL_WARNING ( xcSelfZombie, "negative refcount" );
                break;
            default:
                INTERNAL_WARNING ( xcUnexpected, "unknown result from KRefcountDrop: ( %d )", drop_result );
            }
        }
    }
}


/* Init
 *  initialize the refcount to 1
 *  may perform logging
 */
LIB_EXPORT void CC KRefcountInit_v1 ( KRefcount_v1 *self, ctx_t ctx,
    const KVTable * kvt, const char * instance_name )
{
    FUNC_ENTRY ( ctx, rcRuntime, rcData, rcConstructing );

    if ( self == NULL )
        INTERNAL_ERROR ( xcSelfNull, "failed to construct object" );
    else if ( kvt == NULL )
        INTERNAL_ERROR ( xcInterfaceNull, "null vtable" );
    else
    {
        const KRefcount_v1_vt *vt = KVTABLE_CAST ( kvt, ctx, KRefcount_v1 );
        if ( vt == NULL )
            INTERNAL_ERROR ( xcInterfaceIncorrect, "vtable does not appear to implement KRefcount_v1" );
        else
        {
            switch ( vt -> dad . min )
            {
            case 0:
                break;
            default:
                INTERNAL_ERROR ( xcInterfaceInvalid, "vtable has an invalid minor version" );
                return;
            }

            KRefcountInit ( & self -> refcount, 1, kvt -> name, "init", instance_name ? instance_name : "" );
            self -> vt = kvt;
#if _DEBUGGING && _ARCH_BITS > 32
            memset ( & self -> align, 0, sizeof self -> align );
#endif
        }
    }
}

/* Destroy
 *  has nothing to destroy
 *  may perform logging
 */
LIB_EXPORT void CC KRefcountDestroy_v1 ( KRefcount_v1 *self, ctx_t ctx )
{
#if _DEBUGGING
    if ( self != NULL )
        KRefcountWhack ( & self -> refcount, self -> vt -> name );
#endif
}
