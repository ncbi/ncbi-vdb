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

#include "database-base.h"

#define KDatabase KDatabaseBase
#include <kdb/database.h>

#include <kdb/extern.h>

#include <klib/rc.h>

rc_t KDatabaseBaseBaseWhack ( KDatabase *self )
{
    KRefcountWhack ( & self -> refcount, "KDatabase" );
    free ( self );
    return 0;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
rc_t CC KDatabaseBaseBaseAddRef ( const KDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KDatabase" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcColumn, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

rc_t CC KDatabaseBaseBaseRelease ( const KDatabase *self )
{
    switch ( KRefcountDrop ( & self -> refcount, "KDatabase" ) )
    {
    case krefWhack:
        return self -> vt -> whack ( ( KDatabase* ) self );
    case krefNegative:
        return RC ( rcDB, rcColumn, rcReleasing, rcRange, rcExcessive );
    }
    return 0;
}

/* Attach
 * Sever
 */
KDatabase *KDatabaseAttach ( const KDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KDatabase" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KDatabase* ) self;
}

rc_t KDatabaseSever ( const KDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KDatabase" ) )
        {
        case krefWhack:
            return self -> vt -> whack ( ( KDatabase* ) self );
        case krefNegative:
            return RC ( rcDB, rcColumn, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/******************* dispatch functions ***********************/
#define DISPATCH(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return RC ( rcVDB, rcCursor, rcAccessing, rcSelf, rcNull );
#define DISPATCH_BOOL(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return false;

LIB_EXPORT rc_t CC KDatabaseAddRef ( const KDatabase *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( addRef( self ) );
}
LIB_EXPORT rc_t CC KDatabaseRelease ( const KDatabase *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( release( self ) );
}
