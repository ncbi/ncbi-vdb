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

#define TRACK_REFERENCES 0

#include <kdb/extern.h>
#include "libkdb.vers.h"

#define KONST const
#include "dbmgr-priv.h"
#include "kdb-priv.h"
#undef KONST

#include <vfs/manager.h>
#include <kfs/directory.h>
#include <klib/symbol.h>
#include <klib/checksum.h>
#include <klib/rc.h>
#include <kproc/lock.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>



/*--------------------------------------------------------------------------
 * KDBManager
 *  handle to library
 */


/* Whack
 */
static
rc_t KDBManagerWhack ( KDBManager *self )
{
    rc_t rc;

    KRefcountWhack ( & self -> refcount, "KDBManager" );

    /* everything should be closed */
    assert ( self -> open_objs . root == NULL );

    rc = KRWLockRelease ( self -> open_objs_lock );

    rc = VFSManagerRelease ( self -> vfsmgr );

    rc = KDirectoryRelease ( self -> wd );
    if ( rc == 0 )
    {
        free ( self );
        return 0;
    }

    KRefcountInit ( & self -> refcount, 1, "KDBManager", "whack", "kmgr" );
    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KDBManagerAddRef ( const KDBManager *self )
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

LIB_EXPORT rc_t CC KDBManagerRelease ( const KDBManager *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KDBManager" ) )
        {
        case krefWhack:
            return KDBManagerWhack ( ( KDBManager* ) self );
        case krefNegative:
            return RC ( rcDB, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Attach
 * Sever
 */
KDBManager* KDBManagerAttach ( const KDBManager *self )
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
            return KDBManagerWhack ( ( KDBManager* ) self );
        case krefNegative:
            return RC ( rcDB, rcMgr, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Make - PRIVATE
 *
 *  "wd" [ IN, NULL OKAY ] - optional working directory for
 *  accessing the file system. mgr will attach its own reference.
 */
rc_t KDBManagerMake ( KDBManager **mgrp, const KDirectory *wd, const char *op,
    VFSManager *vmanager )
{
    rc_t rc;

    if ( mgrp == NULL )
        rc = RC ( rcDB, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        KDBManager *mgr = malloc ( sizeof * mgr );
        if ( mgr == NULL )
            rc = RC ( rcDB, rcMgr, rcConstructing, rcMemory, rcExhausted );
        else
        {
            memset ( mgr, 0, sizeof * mgr );
            mgr -> wd = wd;
            if ( wd != NULL )
                rc = KDirectoryAddRef ( wd );
            else
                rc = KDirectoryNativeDir ( ( KDirectory** ) & mgr -> wd );

            if ( rc == 0 )
            {
                if (vmanager == NULL) {
                    rc = VFSManagerMake ( & mgr -> vfsmgr );
                }
                else {
                    rc = VFSManagerAddRef ( vmanager );
                    if (rc == 0) {
                        mgr -> vfsmgr = vmanager;
                    }
                }

                if ( rc == 0 )
                {
                    rc = KRWLockMake ( & mgr -> open_objs_lock );

                    if ( rc == 0 )
                    {
                        CRC32Init ();
                        
                        BSTreeInit ( & mgr -> open_objs );
                        
                        KRefcountInit ( & mgr -> refcount, 1, "KDBManager", op, "kmgr" );
                        
                        * mgrp = mgr;
                        return 0;
                    }

                    VFSManagerRelease ( mgr -> vfsmgr );
                }

                KDirectoryRelease ( mgr -> wd );
            }

            free ( mgr );
        }

        * mgrp = NULL;
    }

    return rc;
}


LIB_EXPORT rc_t CC KDBManagerGetVFSManager ( const KDBManager *self,
    const struct VFSManager **vmanager )
{
    if (self == NULL) {
        return RC ( rcDB, rcMgr, rcAccessing, rcSelf, rcNull );
    }
    else if (vmanager == NULL) {
        return RC ( rcDB, rcMgr, rcAccessing, rcParam, rcNull );
    }
    else {
        rc_t rc = VFSManagerAddRef(self -> vfsmgr);
        if (rc == 0) {
            * vmanager = self -> vfsmgr;
        }
        else {
            * vmanager = NULL;
        }
        return rc;
    }
}


/* Version
 *  returns the library version
 */
LIB_EXPORT rc_t CC KDBManagerVersion ( const KDBManager *self, uint32_t *version )
{
    if ( version == NULL )
        return RC ( rcDB, rcMgr, rcAccessing, rcParam, rcNull );
    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcAccessing, rcSelf, rcNull );

    * version = LIBKDB_VERS;
    return 0;
}


/* Exists
 *  returns true if requested object exists
 *
 *  "requested" [ IN ] - a KDBPathType
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT bool CC KDBManagerVExists ( const KDBManager *self, uint32_t requested, const char *name, va_list args )
{
    int type;

    type = KDBManagerVPathType (self, name, args);
    switch ( type )
    {
    case kptDatabase:
    case kptTable:
    case kptIndex:
    case kptColumn:
    case kptMetadata:
        break;
    case kptPrereleaseTbl:
        type = kptTable;
        break;
    default:
        return false;
    }
    return requested == ( uint32_t ) type;
}

bool KDBManagerExists ( const KDBManager *self, uint32_t type, const char *name, ... )
{
    bool exists;

    va_list args;
    va_start ( args, name );

    exists = KDBManagerVExists ( self, type, name, args );

    va_end ( args );

    return exists;
}


/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "path" [ IN ] - NUL terminated path
 */

/* TBD - should be in this file */

/* CheckOpen
 */
rc_t KDBManagerCheckOpen ( const KDBManager * self, const char *path )
{
    /* try to find the open object by path */
    KSymbol *sym = KDBManagerOpenObjectFind ( self, path );

    /* if not open, return no error */
    if ( sym == NULL )
        return 0;

    /* put together an appropriate response */
    switch ( sym -> type )
    {
    case kptDatabase:
        return RC ( rcDB, rcMgr, rcAccessing, rcDatabase, rcBusy );
    case kptTable:
        return RC ( rcDB, rcMgr, rcAccessing, rcTable, rcBusy );
    case kptColumn:
        return RC ( rcDB, rcMgr, rcAccessing, rcColumn, rcBusy );
    case kptIndex:
        return RC ( rcDB, rcMgr, rcAccessing, rcIndex, rcBusy );
    case kptMetadata:
        return RC ( rcDB, rcMgr, rcAccessing, rcMetadata, rcBusy );
    case kptPrereleaseTbl:
        return RC ( rcDB, rcMgr, rcAccessing, rcTable, rcBusy );
    default:
        break;
    }

    return RC ( rcDB, rcMgr, rcAccessing, rcUnknown, rcBusy );
}



/* OpenObjectFind
 *  return a pointer to the openobject with path
 */
static
KSymbol *KDBManagerOpenObjectFindInt ( const KDBManager * self, String *s )
{
    KSymbol *sym = NULL;
    rc_t rc = KRWLockAcquireShared ( self -> open_objs_lock );
    if ( rc == 0 )
    {
        sym = ( KSymbol* ) BSTreeFind ( & self -> open_objs, s, KSymbolCmp );
        KRWLockUnlock ( self -> open_objs_lock );
    }
    return sym;
}

KSymbol *KDBManagerOpenObjectFind ( const KDBManager * self, const char * path )
{
    String s;
    StringInitCString ( & s, path );
    return KDBManagerOpenObjectFindInt ( self, & s );
}


/* OpenObjectBusy
 *  Is this path to an object in the OpenObjects list?
 */
bool KDBManagerOpenObjectBusy ( const KDBManager *self, const char *path )
{
    return KDBManagerOpenObjectFind ( self, path ) != NULL;
}


/* OpenObjectAdd
 *  Put an object in to the open objects list
 */
rc_t KDBManagerOpenObjectAdd ( KDBManager *self, KSymbol *obj )
{
    KSymbol *exists;
    rc_t rc = KRWLockAcquireExcl ( self -> open_objs_lock );
    if ( rc == 0 )
    {
        rc = BSTreeInsertUnique ( & self -> open_objs,
            & obj -> n, ( BSTNode** ) & exists, KSymbolSort );

        KRWLockUnlock ( self -> open_objs_lock );

        if ( rc != 0 )
        {
            switch ( exists -> type )
            {
            case kptDatabase:
                rc = RC ( rcDB, rcMgr, rcInserting, rcDatabase, rcBusy );
                break;
            case kptTable:
            case kptPrereleaseTbl:
                rc = RC ( rcDB, rcMgr, rcInserting, rcTable, rcBusy );
                break;
            case kptColumn:
                rc = RC ( rcDB, rcMgr, rcInserting, rcColumn, rcBusy );
                break;
            case kptIndex:
                rc = RC ( rcDB, rcMgr, rcInserting, rcIndex, rcBusy );
                break;
            case kptMetadata:
                rc = RC ( rcDB, rcMgr, rcInserting, rcMetadata, rcBusy );
                break;
            default:
                rc = RC ( rcDB, rcMgr, rcInserting, rcUnknown, rcBusy );
            }
        }
    }
    return rc;
}


/* OpenDelete
 *  Take an object out of the open objects list
 */
rc_t KDBManagerOpenObjectDelete ( KDBManager *self, KSymbol *obj )
{
    rc_t rc = 0;
    if ( obj != NULL )
    {
        rc = KRWLockAcquireExcl ( self -> open_objs_lock );
        if ( rc == 0 )
        {
            /* we can expect that the only valid reason for
               "obj" not being unlinked is that it was not in
               the tree. other reasons would be that "obj" was
               corrupt, but in any event, it's not in the tree */
            if ( ! BSTreeUnlink ( & self -> open_objs, & obj -> n ) )

                /* to be truly weird, we could tell what kind of node
                   it was that we didn't find */
                rc = RC ( rcDB, rcMgr, rcRemoving, rcNode, rcNotFound );

            KRWLockUnlock ( self -> open_objs_lock );
        }
    }

    return rc;
}


/* ModDate
 *  return a modification timestamp for table
 */
LIB_EXPORT rc_t CC KDBManagerVGetTableModDate ( const KDBManager *self,
    KTime_t *mtime, const char *path, va_list args )
{
    rc_t rc;

    if ( mtime == NULL )
        rc = RC ( rcDB, rcMgr, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self != NULL )
            return KDBVGetPathModDate ( self -> wd, mtime, path, args );

        rc = RC ( rcDB, rcMgr, rcAccessing, rcSelf, rcNull );
        * mtime = 0;
    }

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerGetTableModDate ( const KDBManager *self,
    KTime_t *mtime, const char *path, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, path );
    rc = KDBManagerVGetTableModDate ( self, mtime, path, args );
    va_end ( args );

    return rc;
}
