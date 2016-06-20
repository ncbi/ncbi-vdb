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

#ifndef _h_kfc_refcount_
#define _h_kfc_refcount_

#ifndef _h_kfc_extern_
#include <kfc/extern.h>
#endif

#ifndef _h_kfc_defs_
#include <kfc/defs.h>
#endif

#ifndef _h_atomic32_
#include <atomic32.h>
#endif

/* normally turned on */
#ifndef INLINE_REFCOUNT
#define INLINE_REFCOUNT 0
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KRefcount_v1
 */
typedef struct KRefcount_v1 KRefcount_v1;

/* Duplicate
 *  create a duplicate reference
 *
 *  "rm" [ IN ] - capabilities to remove ( default 0 )
 */
KFC_EXTERN void * CC KRefcountDuplicate_v1 ( KRefcount_v1 * self, ctx_t ctx, caps_t rm );

/* Release
 *  release a reference
 */
KFC_EXTERN void CC KRefcountRelease_v1 ( KRefcount_v1 * self, ctx_t ctx );

/* TO_REFCOUNT
 *  cast C subclasses to KRefcount_v1
 */
#define TO_REFCOUNT_V1( self ) \
    ( ( KRefcount_v1* ) ( self ) )



/*--------------------------------------------------------------------------
 * KRefcount
 *  signed 32-bit reference counter
 * KDualRef
 *  dual signed/unsigned 16-bit reference counter
 */
typedef atomic32_t KRefcount;
typedef atomic32_t KDualRef;


/* Actions
 *  enum of action indicators
 */
enum KRefcountActions
{
    krefOkay,      /* no action needs to be taken           */
    krefWhack,     /* total refcount zero crossing detected */
    krefZero,      /* owned or dep reference zero crossing  */
    krefLimit,     /* modification exceeds limits           */
    krefNegative   /* negative prior or resulting count     */
};


/* Init
 *  initialize a refcount object
 *
 *  "refcount" [ IN/OUT ] - pointer to object being initialized
 *
 *  "value" [ IN ] - initial refcount value
 *    -OR-
 *  "owned" [ IN ] and "dep" [ IN ] - owned and dependent counts
 *  function will return krefLimit if counts exceed limits
 *
 *  "clsname" [ IN ] - NUL-terminated string stating classname
 *  of object instance being initialized.
 *
 *  "op" [ IN ] - NUL-terminated string stating construction operation
 *
 *  "name" [ IN ] - NUL-terminated name of instance
 */
KFC_EXTERN void CC KRefcountInit ( KRefcount *refcount, int value,
    const char *clsname, const char *op, const char *name );
KFC_EXTERN int CC KDualRefInit ( KDualRef *refcount, int owned, int dep,
    const char *clsname, const char *op, const char *name );


/* Whack
 *  tear down whatever was built up in object
 */
KFC_EXTERN void CC KRefcountWhack ( KRefcount *self, const char *clsname );
KFC_EXTERN void CC KDualRefWhack ( KDualRef *self, const char *clsname );


/* Add
 *  add an owned reference
 *  returns enum value indicating action to be taken
 *
 *  inline implementation always returns krefOkay.
 *
 *  linked-in implementation may return any of the following
 *  codes, depending upon how it was compiled:
 *    krefOkay     : reference was successfully added
 *    krefZero     : reference was added, but prior value was zero
 *    krefLimit    : reference was NOT added, too many references
 *    krefNegative : reference was NOT added, prior value was negative
 */
KFC_EXTERN int CC KRefcountAdd ( const KRefcount *self, const char *clsname );
KFC_EXTERN int CC KDualRefAdd ( const KDualRef *self, const char *clsname );


/* Drop
 *  drop an owned reference
 *  returns enum value indicating action to be taken
 *
 *  inline implementation returns:
 *    krefOkay     : reference was successfully dropped
 *    krefWhack    : all references are gone and object can be collected
 *
 *  linked-in implementation may return any of the following
 *  codes, depending upon how it was compiled:
 *    krefOkay     : reference was successfully dropped
 *    krefWhack    : all references are gone and object can be collected
 *    krefZero     : reference was dropped, and no further owned refernces exist
 *    krefNegative : reference was NOT dropped, as resultant count would be negative
 */
KFC_EXTERN int CC KRefcountDrop ( const KRefcount *self, const char *clsname );
KFC_EXTERN int CC KDualRefDrop ( const KDualRef *self, const char *clsname );


/* AddDep
 *  adds a dependency reference
 *  returns enum value indicating action to be taken
 *
 *  inline implementation always returns krefOkay.
 *
 *  linked-in implementation may return any of the following
 *  codes, depending upon how it was compiled:
 *    krefOkay     : reference was successfully added
 *    krefZero     : reference was added, but prior value was zero
 *    krefLimit    : reference was NOT added, too many references
 *    krefNegative : reference was NOT added, prior value was negative
 */
KFC_EXTERN int CC KRefcountAddDep ( const KRefcount *self, const char *clsname );
KFC_EXTERN int CC KDualRefAddDep ( const KDualRef *self, const char *clsname );


/* DropDep
 *  drop a dependency reference
 *  returns enum value indicating action to be taken
 *
 *  inline implementation returns:
 *    krefOkay     : reference was successfully dropped
 *    krefWhack    : all references are gone and object can be collected
 *
 *  linked-in implementation may return any of the following
 *  codes, depending upon how it was compiled:
 *    krefOkay     : reference was successfully dropped
 *    krefWhack    : all references are gone and object can be collected
 *    krefZero     : reference was dropped, and no further dependent refernces exist
 *    krefNegative : reference was NOT dropped, as resultant count would be negative
 */
KFC_EXTERN int CC KRefcountDropDep ( const KRefcount *self, const char *clsname );
KFC_EXTERN int CC KDualRefDropDep ( const KDualRef *self, const char *clsname );



/*--------------------------------------------------------------------------
 * KRefcount inline implementation
 */

#if INLINE_REFCOUNT

#define KRefcountInit( refcount, value, clsname, op, name ) \
    ( REFNEW_COMMA ( clsname, op, name, refcount, value ) \
      atomic32_set ( refcount, value ) )
#define KRefcountWhack( self, clsname ) \
    REFMSG ( clsname, "whack", self )
#define KRefcountAdd( self, clsname ) \
    ( REFMSG_COMMA ( clsname, "addref", self ) \
      ( atomic32_read ( self ) < 0 ) ? krefNegative : \
      ( atomic32_inc ( ( KRefcount* ) ( self ) ), krefOkay ) )
#define KRefcountDrop( self, clsname ) \
    ( REFMSG_COMMA ( clsname, "release", self ) \
      ( atomic32_read ( self ) <= 0 ) ? krefNegative : \
      ( atomic32_dec_and_test ( ( KRefcount* ) ( self ) ) ? krefWhack : krefOkay ) )
#define KRefcountAddDep( self, clsname ) \
    ( REFMSG_COMMA ( clsname, "attach", self ) \
      ( atomic32_read ( self ) < 0 ) ? krefNegative : \
      ( atomic32_inc ( ( KRefcount* ) ( self ) ), krefOkay ) )
#define KRefcountDropDep( self, clsname ) \
    ( REFMSG_COMMA ( clsname, "sever", self ) \
      ( atomic32_read ( self ) <= 0 ) ? krefNegative : \
      ( atomic32_dec_and_test ( ( KRefcount* ) ( self ) ) ? krefWhack : krefOkay ) )

#endif

#define REFNEW( clsname, op, name, instance, refcount ) \
    ( void ) 0
#define CNTMSG( clsname, op, instance, refcount ) \
    ( void ) 0
#define REFMSG( clsname, op, instance ) \
    ( void ) 0
#define REFNEW_COMMA( clsname, op, name, instance, refcount )
#define REFMSG_COMMA( clsname, op, instance )

#ifdef __cplusplus
}
#endif

#endif /* _h_kfc_refcount_ */
