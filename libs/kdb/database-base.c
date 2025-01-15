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

#define KDatabase KDatabaseBase

#include "database-base.h"

#include <kdb/database.h>
#include <kdb/meta.h>

#include <klib/rc.h>

rc_t KDatabaseBaseWhack ( KDatabase *self )
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
rc_t CC KDatabaseBaseAddRef ( const KDatabase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KDatabase" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcDatabase, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

rc_t CC KDatabaseBaseRelease ( const KDatabase *self )
{
    switch ( KRefcountDrop ( & self -> refcount, "KDatabase" ) )
    {
    case krefWhack:
        return self -> vt -> whack ( ( KDatabase* ) self );
    case krefNegative:
        return RC ( rcDB, rcDatabase, rcReleasing, rcRange, rcExcessive );
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
            return RC ( rcDB, rcDatabase, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/******************* dispatch functions ***********************/
#define DISPATCH(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );
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
LIB_EXPORT bool CC KDatabaseLocked ( const KDatabase *self )
{
    DISPATCH_BOOL( locked( self ) );
}
LIB_EXPORT bool CC KDatabaseVExists ( const KDatabase *self, uint32_t type, const char *name, va_list args )
{
    DISPATCH_BOOL( vExists( self, type, name, args ) );
}
LIB_EXPORT bool CC KDatabaseExists ( const KDatabase *self, uint32_t type, const char *name, ... )
{
    bool exists;

    va_list args;
    va_start ( args, name );

    exists = KDatabaseVExists ( self, type, name, args );

    va_end ( args );

    return exists;
}
LIB_EXPORT bool CC KDatabaseIsAlias ( const KDatabase *self, uint32_t type, char *resolved, size_t rsize, const char *name )
{
    DISPATCH_BOOL( isAlias ( self, type, resolved, rsize, name ) );
}
LIB_EXPORT rc_t CC KDatabaseVWritable ( const KDatabase *self, uint32_t type, const char *name, va_list args )
{
    DISPATCH(  vWritable( self, type, name, args ) );
}
LIB_EXPORT rc_t CC KDatabaseWritable ( const KDatabase *self, uint32_t type, const char *name, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, name );

    rc = KDatabaseVWritable ( self, type, name, args );

    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KDatabaseOpenManagerRead ( const KDatabase *self, const struct KDBManager **mgr )
{
    DISPATCH(  openManagerRead( self, mgr ) );
}
LIB_EXPORT rc_t CC KDatabaseOpenParentRead ( const KDatabase *self, const KDatabase **par )
{
    DISPATCH(  openParentRead( self, par ) );
}
LIB_EXPORT rc_t CC KDatabaseOpenDirectoryRead ( const KDatabase *self, const KDirectory **dir )
{
    DISPATCH(  openDirectoryRead( self, dir ) );
}
LIB_EXPORT rc_t CC KDatabaseVOpenDBRead ( const KDatabase *self, const KDatabase **dbp, const char *name, va_list args )
{
    DISPATCH(  vOpenDBRead( self, dbp, name, args ) );
}
LIB_EXPORT rc_t CC KDatabaseOpenDBRead ( const KDatabase *self, const KDatabase **db, const char *name, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVOpenDBRead ( self, db, name, args );
    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KDatabaseVOpenTableRead ( const KDatabase *self, const struct KTable **tblp, const char *name, va_list args )
{
    DISPATCH( vOpenTableRead ( self, tblp, name, args ) );
}
LIB_EXPORT rc_t CC KDatabaseOpenTableRead ( const KDatabase *self,
    const struct KTable **tbl, const char *name, ... )
{
    if ( self == NULL && self -> vt == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcSelf, rcNull );

    rc_t rc;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVOpenTableRead ( self, tbl, name, args );
    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KDatabaseOpenMetadataRead ( const KDatabase *self, const KMetadata **metap )
{
    DISPATCH( openMetadataRead ( self, metap ) );
}
LIB_EXPORT rc_t CC KDatabaseVOpenIndexRead ( const KDatabase *self, const struct KIndex **idxp, const char *name, va_list args )
{
    DISPATCH( vOpenIndexRead ( self, idxp, name, args ) );
}
LIB_EXPORT rc_t CC KDatabaseOpenIndexRead ( struct KDatabase const *self, const struct KIndex **idx, const char *name, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVOpenIndexRead ( self, idx, name, args );
    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KDatabaseListDB ( const KDatabase *self, struct KNamelist **names )
{
    DISPATCH( listDB ( self, names ) );
}
LIB_EXPORT rc_t CC KDatabaseListTbl ( struct KDatabase const *self, struct KNamelist **names )
{
    DISPATCH( listTbl ( self, names ) );
}
LIB_EXPORT rc_t CC KDatabaseListIdx ( struct KDatabase const *self, struct KNamelist **names )
{
    DISPATCH( listIdx ( self, names ) );
}
LIB_EXPORT rc_t CC KDatabaseGetPath ( KDatabase const *self, const char **path )
{
    DISPATCH( getPath ( self, path ) );
}
