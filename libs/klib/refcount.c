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

#define TRACK_REFERENCES 1
#define INLINE_REFCOUNT 0

#include <kfc/extern.h>

#include <klib/refcount.h>
#include <klib/log.h>
#include <atomic32.h>

#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* the simple atomic32_dec_and_test interface does not detect
   crossing to negative. better said, it internally detects the
   event but does not pay for setting a tri-state return value
   on exit, opting instead to only detect the zero crossing. */
#ifndef DETECT_ZERO_STATE
#define DETECT_ZERO_STATE 1
#endif

/* the simple atomic32_inc interface does not detect rollover */
#ifndef DETECT_LIMIT_VIOLATION
#define DETECT_LIMIT_VIOLATION 1
#endif

/* this is in here to detect if the refcount is above a certain value 
   introduced to debug heap corruption in the windows-build */
#define MAX_REFCOUNT_VALUE 1000
#ifndef DETECT_MAX_REFCOUNT_VIOLATION
#define DETECT_MAX_REFCOUNT_VIOLATION 1
#endif


#define DUAL_OWN_BITS 16
#define DUAL_DEP_BITS ( 32 - DUAL_OWN_BITS )
#define DUAL_OWN_VAL ( 1 << DUAL_DEP_BITS )
#define DUAL_DEP_VAL 1
#define DUAL_OWN_MAX ( ( 1 << ( DUAL_OWN_BITS - 1 ) ) - 1 )
#define DUAL_DEP_MAX ( ( 1 << ( DUAL_DEP_BITS - 1 ) ) - 1 )
#define DUAL_OWN_LIM ( DUAL_OWN_MAX << DUAL_DEP_BITS )
#define DUAL_DEP_LIM DUAL_DEP_MAX
#define DUAL_OWN_MASK ( ( 1 << DUAL_OWN_BITS ) - 1 )
#define DUAL_DEP_MASK ( ( 1 << DUAL_DEP_BITS ) - 1 )

/*--------------------------------------------------------------------------
 * KRefcount
 *  signed 32-bit reference counter
 * KDualRef
 *  dual signed/unsigned 16-bit reference counter
 */


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
#if ! INLINE_REFCOUNT
void CC KRefcountInit ( KRefcount *refcount, int value,
    const char *clsname, const char *op, const char *name )
{
    REFNEW ( clsname, op, name, refcount, value );
    atomic32_set ( refcount, value );
}
#endif

int CC KDualRefInit ( KDualRef *refcount, int owned, int dep,
    const char *clsname, const char *op, const char *name )
{
    int value = owned << DUAL_DEP_BITS | dep;

#if DETECT_LIMIT_VIOLATION
    if ( owned < 0 || owned > DUAL_OWN_MAX ||
         dep < 0 || dep > DUAL_DEP_MAX )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED TO CREATE %s, operation %s, name '%s', instance $0x%p: initial refcounts 0x%x, 0x%x",
                   clsname, op, name, refcount, owned, dep ) );
        return krefLimit;
    }
#endif

    REFNEW ( clsname, op, name, refcount, value );
    atomic32_set ( refcount, value );
    return krefOkay;
}


/* Whack
 *  tear down whatever was built up in object
 */
#if ! INLINE_REFCOUNT
void CC KRefcountWhack ( KRefcount *self, const char *clsname )
{
    REFMSG ( clsname, "whack", self );
}
#endif

void CC KDualRefWhack ( KDualRef *self, const char *clsname )
{
    REFMSG ( clsname, "whack", self );
}


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
#if ! INLINE_REFCOUNT
int CC KRefcountAdd ( const KRefcount *self, const char *clsname )
{
#if DETECT_ZERO_STATE
    int prior = atomic32_read_and_add_ge ( ( KRefcount* ) self, 1, 0 );
    if ( prior < 0 )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to addref %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefNegative;
    }
    if ( prior == 0 )
    {
#if 0 /*** disabling this warning since code is noe initializing refcount to zero in several places ***/
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "about to addref %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
#endif
        return krefZero;
    }

#if DETECT_LIMIT_VIOLATION
    if ( prior == INT_MAX )
    {
        atomic32_dec ( ( KRefcount* ) self );
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to addref %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefLimit;
    }
#endif

    CNTMSG ( clsname, "addref", self, prior );

#elif DETECT_LIMIT_VIOLATION
    unsigned int prior = atomic32_read_and_add_lt ( ( KRefcount* ) self, 1, INT_MAX );
    if ( prior >= INT_MAX )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to addref %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefLimit;
    }
    CNTMSG ( clsname, "addref", self, prior );
#else
    REFMSG ( clsname, "addref", self );
    atomic32_inc ( ( KRefcount* ) ( self ) );
#endif

    return krefOkay;
}
#endif

int CC KDualRefAdd ( const KDualRef *self, const char *clsname )
{
#if DETECT_LIMIT_VIOLATION
    unsigned int prior = atomic32_read_and_add_lt ( ( KDualRef* ) self, DUAL_OWN_VAL, DUAL_OWN_LIM );
#if DETECT_ZERO_STATE
    unsigned int owned = prior >> DUAL_DEP_BITS;
    if ( owned > DUAL_OWN_MAX )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER, 
                  ( "FAILED to addref %s instance 0x%p: prior refcount = 0x%x",
                    clsname, self, prior ));
        return krefNegative;
    }
    if ( prior == 0 )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER, 
                 ( "about to addref %s instance 0x%p: prior refcount = 0x%x",
                    clsname, self, prior ) );
        return krefZero;
    }
#endif
    if ( prior >= DUAL_OWN_LIM )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER, 
                  ( "FAILED to addref %s instance 0x%p: prior refcount = 0x0%x",
                    clsname, self, prior ));
        return krefLimit;
    }
    CNTMSG ( clsname, "addref", self, prior );
#elif DETECT_ZERO_STATE
    unsigned int prior = atomic32_read_and_add ( ( KDualRef* ) self, DUAL_OWN_VAL );
    unsigned int owned = prior >> DUAL_DEP_BITS;
    if ( owned > DUAL_OWN_MAX )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                  ( "FAILED to addref %s instance %0x%p: prior refcount = 0x%x",
                    clsname, self, prior ));
        return krefNegative;
    }
    if ( prior == 0 )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                  ( "about to addref %s instance %0x%p: prior refcount = 0x%x",
                    clsname, self, prior ));
        return krefZero;
    }
    CNTMSG ( clsname, "addref", self, prior );
#else
    REFMSG ( clsname, "addref", self );
    atomic32_add ( ( KDualRef* ) ( self ), DUAL_OWN_VAL );
#endif
    return krefOkay;
}


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
#if ! INLINE_REFCOUNT
int CC KRefcountDrop ( const KRefcount *self, const char *clsname )
{
#if DETECT_ZERO_STATE
    int prior = atomic32_read_and_add ( ( KRefcount* ) self, -1 );
    if ( prior <= 0 )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to release %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefNegative;
    }
    CNTMSG ( clsname, "release", self, prior );
    if ( prior == 1 )
        return krefWhack;
#else
    REFMSG ( clsname, "release", self );
    if ( atomic32_dec_and_test ( ( KRefcount* ) ( self ) ) )
        return krefWhack;
#endif
    return krefOkay;
}
#endif

int CC KDualRefDrop ( const KDualRef *self, const char *clsname )
{
    int prior = atomic32_read_and_add_ge ( ( KDualRef* ) self, - DUAL_OWN_VAL, DUAL_OWN_VAL );
#if DETECT_ZERO_STATE
    if ( prior < DUAL_OWN_VAL )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to release %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefNegative;
    }
#endif
    CNTMSG ( clsname, "release", self, prior );
    if ( prior == DUAL_OWN_VAL )
        return krefWhack;
    if ( ( prior >> DUAL_DEP_BITS ) == 0 )
        return krefZero;
    return krefOkay;
}


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
 *    krefNegative : reference was NOT added, prior value was negative
 */
#if ! INLINE_REFCOUNT
int CC KRefcountAddDep ( const KRefcount *self, const char *clsname )
{
#if DETECT_ZERO_STATE
    int prior = atomic32_read_and_add ( ( KRefcount* ) self, 1 );
    if ( prior < 0 )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to attach %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefNegative;
    }
    if ( prior == 0 )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "about to attach %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefZero;
    }
#if DETECT_LIMIT_VIOLATION
    if ( prior == INT_MAX )
    {
        atomic32_dec ( ( KRefcount* ) self );
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to attach %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefLimit;
    }
#endif
    CNTMSG ( clsname, "attach", self, prior );
#elif DETECT_LIMIT_VIOLATION
    unsigned int prior = atomic32_read_and_add_lt ( ( KRefcount* ) self, 1, INT_MAX );
    if ( prior >= INT_MAX )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to attach %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefLimit;
    }
    CNTMSG ( clsname, "attach", self, prior );
#else
    REFMSG ( clsname, "attach", self );
    atomic32_inc ( ( KRefcount* ) ( self ) );
#endif
    return krefOkay;
}
#endif

int CC KDualRefAddDep ( const KDualRef *self, const char *clsname )
{
#if DETECT_ZERO_STATE
    int prior = atomic32_read_and_add_ge ( ( KDualRef* ) self, 1, 0 );
    int dep = prior & DUAL_DEP_MASK;
    if ( prior < 0 || dep > DUAL_DEP_MAX )
    {
        if ( prior >= 0 )
            atomic32_dec ( ( KDualRef* ) self );
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to attach %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefNegative;
    }
    if ( prior == 0 )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "about to attach %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefZero;
    }
#if DETECT_LIMIT_VIOLATION
    if ( dep == DUAL_DEP_MAX )
    {
        atomic32_dec ( ( KDualRef* ) self );
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to attach %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefLimit;
    }
#endif
    CNTMSG ( clsname, "attach", self, prior );
#elif DETECT_LIMIT_VIOLATION
    int prior = atomic32_read_and_inc ( ( KDualRef* ) self );
    if ( ( prior & DUAL_DEP_MASK ) >= DUAL_DEP_MAX )
    {
        atomic32_dec ( ( KDualRef* ) self );
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to attach %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefLimit;
    }
    CNTMSG ( clsname, "attach", self, prior );
#else
    REFMSG ( clsname, "attach", self );
    atomic32_inc ( ( KDualRef* ) ( self ) );
#endif
    return krefOkay;
}


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
#if ! INLINE_REFCOUNT
int CC KRefcountDropDep ( const KRefcount *self, const char *clsname )
{
#if DETECT_ZERO_STATE
    int prior = atomic32_read_and_add ( ( KRefcount* ) self, -1 );
    if ( prior <= 0 )
    {
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to sever %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefNegative;
    }
    CNTMSG ( clsname, "sever", self, prior );
    if ( prior == 1 )
        return krefWhack;
#else
    REFMSG ( clsname, "sever", self );
    if ( atomic32_dec_and_test ( ( KRefcount* ) ( self ) ) )
        return krefWhack;
#endif
    return krefOkay;
}
#endif

int CC KDualRefDropDep ( const KDualRef *self, const char *clsname )
{
    int prior = atomic32_read_and_add ( ( KDualRef* ) self, -1 );
#if DETECT_ZERO_STATE
    if ( prior <= 0 )
    {
        atomic32_inc ( ( KDualRef* ) self );
        DBGMSG ( DBG_REF, DBG_REF_PLACEHOLDER,
                 ( "FAILED to sever %s instance 0x%p: prior refcount = 0x%x",
                   clsname, self, prior ) );
        return krefNegative;
    }
#endif
    CNTMSG ( clsname, "sever", self, prior );
    if ( prior == 1 )
        return krefWhack;
    if ( ( prior & DUAL_DEP_MASK ) == 1 )
        return krefZero;
    return krefOkay;
}
