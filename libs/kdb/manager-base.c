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

#include <kdb/extern.h>

#define KDBManager KDBManagerBase

#include "manager-base.h"

#include <kdb/database.h>

#include <klib/rc.h>

struct KDatabase;

rc_t KDBManagerBaseWhack ( KDBManager *self )
{
    KRefcountWhack ( & self -> refcount, "KDBManager" );
    free ( self );
    return 0;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
rc_t CC KDBManagerBaseAddRef ( const KDBManager *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KDBManager" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcMgr, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

rc_t CC KDBManagerBaseRelease ( const KDBManager *self )
{
    switch ( KRefcountDrop ( & self -> refcount, "KDBManager" ) )
    {
    case krefWhack:
        return self -> vt -> whack ( ( KDBManager* ) self );
    case krefNegative:
        return RC ( rcDB, rcMgr, rcReleasing, rcRange, rcExcessive );
    }
    return 0;
}

/* Attach
 * Sever
 */
KDBManager *KDBManagerAttach ( const KDBManager *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KDBManager" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KDBManager* ) self;
}

rc_t KDBManagerSever ( const KDBManager *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KDBManager" ) )
        {
        case krefWhack:
            return self -> vt -> whack ( ( KDBManager* ) self );
        case krefNegative:
            return RC ( rcDB, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/******************* dispatch functions ***********************/
#define DISPATCH(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return RC ( rcDB, rcMgr, rcAccessing, rcSelf, rcNull );
#define DISPATCH_BOOL(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return false;

LIB_EXPORT rc_t CC KDBManagerAddRef ( const KDBManager *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( addRef( self ) );
}
LIB_EXPORT rc_t CC KDBManagerRelease ( const KDBManager *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( release( self ) );
}
LIB_EXPORT rc_t CC KDBManagerVersion ( const KDBManager *self, uint32_t *version )
{
    DISPATCH( version( self, version ) );
}
LIB_EXPORT bool CC KDBManagerVExists ( const KDBManager *self, uint32_t requested, const char *name, va_list args )
{
    DISPATCH_BOOL( vExists( self, requested, name, args ) );
}
LIB_EXPORT bool KDBManagerExists ( const KDBManager *self, uint32_t type, const char *name, ... )
{
    bool exists;

    va_list args;
    va_start ( args, name );

    exists = KDBManagerVExists ( self, type, name, args );

    va_end ( args );

    return exists;
}
LIB_EXPORT rc_t CC KDBManagerVWritable ( const KDBManager *self, const char * path, va_list args )
{
    DISPATCH( vWritable( self, path, args ) );
}
LIB_EXPORT rc_t CC KDBManagerWritable ( const KDBManager *self, const char * path, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, path );

    rc = KDBManagerVWritable ( self, path, args );

    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KDBManagerRunPeriodicTasks ( const KDBManager *self )
{
    DISPATCH( runPeriodicTasks( self ) );
}
LIB_EXPORT int CC KDBManagerPathTypeVP ( const KDBManager * self, const struct VPath * path )
{
    DISPATCH( pathTypeVP( self, path ) );
}
LIB_EXPORT int CC KDBManagerVPathType ( const KDBManager * self, const char *path, va_list args )
{
    DISPATCH( vPathType( self, path, args ) );
}
LIB_EXPORT int CC KDBManagerPathType ( const KDBManager * self, const char *path, ... )
{
    int res;
    va_list args;

    va_start ( args, path );

    res = KDBManagerVPathType ( self, path, args );

    va_end (args);
    return res;
}
LIB_EXPORT int CC KDBManagerVPathTypeUnreliable ( const KDBManager * self, const char *path, va_list args )
{
    DISPATCH( vPathTypeUnreliable( self, path, args ) );
}
LIB_EXPORT rc_t CC KDBManagerVOpenDBRead ( const KDBManager *self, const KDatabase **db, const char *path, va_list args )
{
    DISPATCH( vOpenDBRead( self, db, path, args, NULL ) );
}
LIB_EXPORT rc_t CC KDBManagerOpenDBRead ( const KDBManager *self, const KDatabase **db, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenDBRead ( self, db, path, args );
    va_end ( args );

    return rc;
}
static rc_t openDBReadVPath(const KDBManager *self
                            , const KDatabase **db
                            , const struct VPath *path
                            , ... )
{
    if (self) {
        rc_t rc = 0;
        va_list va;
        va_start(va, path);
        rc = self->vt->vOpenDBRead(self, db, NULL, va, path);
        va_end(va);
        return rc;
    }
    else
        return RC ( rcDB, rcMgr, rcAccessing, rcSelf, rcNull );
}
LIB_EXPORT rc_t CC KDBManagerOpenDBReadVPath ( const KDBManager *self,
    const KDatabase **db, const struct VPath *path )
{
    return openDBReadVPath(self, db, path);
}
LIB_EXPORT rc_t CC KDBManagerVOpenTableRead ( const KDBManager *self, const struct KTable **tbl, const char *path, va_list args )
{
    DISPATCH( vOpenTableRead( self, tbl, path, args ) );
}
LIB_EXPORT rc_t CC KDBManagerOpenTableRead ( const KDBManager *self, const struct KTable **tbl, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenTableRead ( self, tbl, path, args );
    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KDBManagerOpenTableReadVPath ( const KDBManager *self, const struct KTable **tbl, const struct VPath *path )
{
    DISPATCH( openTableReadVPath( self, tbl, path ) );
}
LIB_EXPORT rc_t CC KDBManagerVOpenColumnRead ( const KDBManager *self, const KColumn **col, const char *path, va_list args )
{
    DISPATCH( vOpenColumnRead( self, col, path, args ) );
}
LIB_EXPORT rc_t CC KDBManagerOpenColumnRead ( const KDBManager *self, const KColumn **col, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenColumnRead ( self, col, path, args );
    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KDBManagerVPathOpenLocalDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * vpath )
{
    DISPATCH( vPathOpenLocalDBRead( self, p_db, vpath ) );
}
LIB_EXPORT rc_t CC KDBManagerVPathOpenRemoteDBRead ( struct KDBManager const * self,
    struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache )
{
    DISPATCH( vPathOpenRemoteDBRead( self, p_db, remote, cache ) );
}

LIB_EXPORT rc_t CC KDBManagerGetVFSManager ( struct KDBManager const *self, const struct VFSManager ** vfs )
{
    DISPATCH( getVFSManager( self, vfs ) );
}

