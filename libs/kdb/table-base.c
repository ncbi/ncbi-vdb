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

#include "table-base.h"

#include <kdb/table.h>

#include <kdb/column.h>
#include <kdb/meta.h>

#include <klib/rc.h>

rc_t KTableBaseWhack ( KTable *self )
{
    KRefcountWhack ( & self -> refcount, "KTable" );
    free ( self );
    return 0;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
rc_t CC KTableBaseAddRef ( const KTable *self )
{
    switch ( KRefcountAdd ( & self -> refcount, "KTable" ) )
    {
    case krefLimit:
        return RC ( rcDB, rcTable, rcAttaching, rcRange, rcExcessive );
    }
    return 0;
}

rc_t CC KTableBaseRelease ( const KTable *self )
{
    switch ( KRefcountDrop ( & self -> refcount, "KTable" ) )
    {
    case krefWhack:
        return self -> vt -> whack ( ( KTable* ) self );
    case krefNegative:
        return RC ( rcDB, rcTable, rcReleasing, rcRange, rcExcessive );
    }
    return 0;
}

/* Attach
 * Sever
 */
KTable *KTableAttach ( const KTable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KTable" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KTable* ) self;
}

rc_t KTableSever ( const KTable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KTable" ) )
        {
        case krefWhack:
            return self -> vt -> whack ( ( KTable* ) self );
        case krefNegative:
            return RC ( rcDB, rcTable, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}

/******************* dispatch functions ***********************/
#define DISPATCH(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return RC ( rcDB, rcTable, rcAccessing, rcSelf, rcNull );
#define DISPATCH_BOOL(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return false;

LIB_EXPORT rc_t CC KTableAddRef ( const KTable *self )
{
    DISPATCH( addRef( self ) );
}
LIB_EXPORT rc_t CC KTableRelease ( const KTable *self )
{
    if ( self == NULL )
        return 0;
    DISPATCH( release( self ) );
}
LIB_EXPORT bool CC KTableLocked ( const KTable *self )
{
    DISPATCH_BOOL( locked( self ) );
}
LIB_EXPORT bool CC KTableVExists ( const KTable *self, uint32_t type,
    const char *name, va_list args )
{
    DISPATCH_BOOL( vExists( self, type, name, args ) );
}
LIB_EXPORT bool CC KTableExists ( const KTable *self, uint32_t type, const char *name, ... )
{
    bool exists;

    va_list args;
    va_start ( args, name );

    exists = KTableVExists ( self, type, name, args );

    va_end ( args );

    return exists;
}
LIB_EXPORT bool CC KTableIsAlias ( const KTable *self, uint32_t type, char *resolved, size_t rsize, const char *name )
{
    DISPATCH_BOOL( isAlias( self, type, resolved, rsize, name ) );
}
LIB_EXPORT rc_t CC KTableVWritable ( const KTable *self, uint32_t type,
    const char *name, va_list args )
{
    DISPATCH( vWritable( self, type, name, args ) );
}
LIB_EXPORT rc_t CC KTableWritable ( const KTable *self, uint32_t type, const char *name, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, name );

    rc = KTableVWritable ( self, type, name, args );

    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KTableOpenManagerRead ( const KTable *self, struct KDBManager const **mgr )
{
    DISPATCH( openManagerRead( self, mgr ) );
}
LIB_EXPORT rc_t CC KTableOpenParentRead ( const KTable *self, const struct KDatabase **db )
{
    DISPATCH( openParentRead( self, db ) );
}
LIB_EXPORT bool CC KTableHasRemoteData ( const KTable *self )
{
    DISPATCH_BOOL( hasRemoteData( self ) );
}
LIB_EXPORT rc_t CC KTableOpenDirectoryRead ( const KTable *self, const KDirectory **dir )
{
    DISPATCH( openDirectoryRead( self, dir ) );
}
LIB_EXPORT rc_t CC KTableVOpenColumnRead ( const KTable *self,
    const KColumn **colp, const char *name, va_list args )
{
    DISPATCH( vOpenColumnRead( self, colp, name, args ) );
}
LIB_EXPORT rc_t CC KTableOpenColumnRead ( const KTable *self,
    const KColumn **col, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KTableVOpenColumnRead ( self, col, path, args );
    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KTableOpenMetadataRead ( const KTable *self, const KMetadata **metap )
{
    DISPATCH( openMetadataRead( self, metap ) );
}
LIB_EXPORT rc_t CC KTableVOpenIndexRead ( struct KTable const *self, const struct KIndex **idx, const char *name, va_list args )
{
    DISPATCH( vOpenIndexRead( self, idx, name, args ) );
}
LIB_EXPORT rc_t CC KTableOpenIndexRead ( struct KTable const *self, const struct KIndex **idx, const char *name, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, name );
    rc = KTableVOpenIndexRead ( self, idx, name, args );
    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KTableGetPath ( const KTable *self, const char **path )
{
    DISPATCH( getPath( self, path ) );
}
LIB_EXPORT rc_t KTableGetName( struct KTable const *self, char const **rslt)
{
    DISPATCH( getName( self, rslt ) );
}
LIB_EXPORT rc_t CC KTableListCol ( const KTable *self, struct KNamelist **names )
{
    DISPATCH( listCol( self, names ) );
}
LIB_EXPORT rc_t CC KTableListIdx ( const KTable *self, struct KNamelist **names )
{
    DISPATCH( listIdx( self, names ) );
}
LIB_EXPORT rc_t CC KTableMetaCompare( const KTable *self, const KTable *other, const char * path, bool * equal )
{
    DISPATCH( metaCompare( self, other, path, equal ) );
}
