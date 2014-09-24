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

#ifndef _h_kfc_vtbl_impl_
#define _h_kfc_vtbl_impl_

#ifndef _h_kfc_extern_
#include <kfc/extern.h>
#endif

#ifndef _h_kfc_defs_
#include <kfc/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KVTable
 */
typedef struct KVTable KVTable;

typedef struct KItfTok KItfTok;
struct KItfTok
{
    const char * const _name;
    uint32_t idx;
};

/* use this macro to declare an extern */
#define KITFTOK_DECL( itf_name ) \
    KItfTok itf_name ## _tok

/* use this macro to define the token */
#define KITFTOK_DEF( itf_name ) \
    KItfTok itf_name ## _tok = { #itf_name }


typedef struct KHierCache KHierCache;
struct KHierCache
{
    const KHierCache * volatile next;

    size_t length;
    const KVTable * parent [ 1 ];
};

struct KVTable
{
    /* implementation classname */
    const char * name;

    /* external token
       identifies interface class */
    const KItfTok * itf;

    /* minor version number
       allows for extensions */
    size_t min;

    /* single-inheritance
       pointer to parent */
    const KVTable * parent;

    /* cached linear inheritance array */
    const KHierCache * cache;
};

#define KVTABLE_INITIALIZER( impl_name, itf_name, min_vers, parent_vt )   \
    { #impl_name, & itf_name ## _tok, min_vers, parent_vt }


/* Resolve
 *  walks a vtable's hierarchy
 *  builds a linear array cache of casting pointers
 *  dynamically configures KItfTok objects to cache their runtime offset
 */
void KVTableResolve ( const KVTable * self, ctx_t ctx );


/* Cast
 *  casts a vtable to a particular interface
 *  returns NULL if not found
 */
static __inline__
const void * KVTableCast ( const KVTable * self, ctx_t ctx, const KItfTok * itf )
{
    if ( self != NULL )
    {
        if ( self -> cache == NULL )
            KVTableResolve ( self, ctx );

        assert ( itf -> idx != 0 );
        assert ( itf -> idx <= ( uint32_t ) self -> cache -> length );

#if KVTABLE_NO_RTT_CHECK
        return self -> cache -> parent [ itf -> idx - 1 ];
#else
        self = self -> cache -> parent [ itf -> idx - 1 ];
        if ( self -> itf == itf )
            return self;
#endif
    }
    return NULL;
}
#define KVTABLE_CAST( vt, ctx, itf_name ) \
    KVTableCast ( vt, ctx, & itf_name ## _tok )


#ifdef __cplusplus
}
#endif

#endif /* _h_kfc_vtbl_impl_ */
