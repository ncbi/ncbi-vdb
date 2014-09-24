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

#include <kproc/extern.h>

#include <kproc/task.h>
#include <kproc/impl.h>
#include <klib/rc.h>

#define rcTask rcCmd


/*--------------------------------------------------------------------------
 * KTask
 *  a deferred task abstraction
 *  specific task objects are constructed with required parameters
 *  and implement the Execute method to perform their operation
 */


/* Whack
 */
static
rc_t KTaskWhack ( KTask * self )
{
    if ( self == NULL )
        return RC ( rcPS, rcTask, rcDestroying, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . destroy ) ( self );
    }

    return RC ( rcPS, rcTask, rcDestroying, rcInterface, rcBadVersion );
}


/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC KTaskAddRef ( const KTask *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KTask" ) )
        {
        case krefLimit:
            return RC ( rcPS, rcTask, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KTaskRelease ( const KTask *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KTask" ) )
        {
        case krefWhack:
            return KTaskWhack ( ( KTask* ) self );
        case krefNegative:
            return RC ( rcPS, rcTask, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Execute
 *  perform deferred operation
 */
LIB_EXPORT rc_t CC KTaskExecute ( KTask *self )
{
    if ( self == NULL )
        return RC ( rcPS, rcTask, rcExecuting, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . execute ) ( self );
    }

    return RC ( rcPS, rcTask, rcExecuting, rcInterface, rcBadVersion );
}


/* Init
 *  initialize a newly allocated task object
 */
LIB_EXPORT rc_t CC KTaskInit ( KTask *self, const KTask_vt *vt, const char *clsname, const char *name )
{
    if ( self == NULL )
        return RC ( rcFS, rcTask, rcConstructing, rcSelf, rcNull );
    if ( vt == NULL )
        return RC ( rcFS, rcTask, rcConstructing, rcInterface, rcNull );
    switch ( vt -> v1 . maj )
    {
    case 1:
        switch ( vt -> v1 . min )
        {
            /* insert new minor versions in reverse order */
        case 0:
#if _DEBUGGING
            if ( vt -> v1 . destroy == NULL ||
                 vt -> v1 . execute == NULL )
            return RC ( rcFS, rcTask, rcConstructing, rcInterface, rcNull );
#endif
            break;
        default:
            return RC ( rcFS, rcTask, rcConstructing, rcInterface, rcInvalid );
        }
        break;
    default:
        return RC ( rcFS, rcTask, rcConstructing, rcInterface, rcInvalid );
    }

    self -> vt = vt;
    KRefcountInit ( & self -> refcount, 1, clsname, "init", name );
    return 0;
}


/* Destroy
 *  destroy task
 */
LIB_EXPORT rc_t CC KTaskDestroy ( KTask *self, const char *clsname )
{
    if ( self != NULL )
        KRefcountWhack ( & self -> refcount, clsname );
    return 0;
}
