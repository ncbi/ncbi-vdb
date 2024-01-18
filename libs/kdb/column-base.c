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


#include "column-base.h"

#include <kdb/column.h>

#include <kdb/meta.h>

#include <klib/rc.h>

rc_t KColumnBaseWhack ( KColumn *self )
{
    KRefcountWhack ( & self -> refcount, "KColumn" );
    free ( self );
    return 0;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
rc_t CC KColumnBaseAddRef ( const KColumn *self )
{
    switch ( KRefcountAdd ( & self -> refcount, "KColumn" ) )
    {
    case krefLimit:
        return RC ( rcDB, rcColumn, rcAttaching, rcRange, rcExcessive );
    }
    return 0;
}

rc_t CC KColumnBaseRelease ( const KColumn *self )
{
    switch ( KRefcountDrop ( & self -> refcount, "KColumn" ) )
    {
    case krefWhack:
        return self -> vt -> whack ( ( KColumn* ) self );
    case krefNegative:
        return RC ( rcDB, rcColumn, rcReleasing, rcRange, rcExcessive );
    }
    return 0;
}

/* Attach
 * Sever
 */
KColumn *KColumnAttach ( const KColumn *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KColumn" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KColumn* ) self;
}

rc_t KColumnSever ( const KColumn *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KColumn" ) )
        {
        case krefWhack:
            return self -> vt -> whack ( ( KColumn* ) self );
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
        return RC ( rcDB, rcColumn, rcAccessing, rcSelf, rcNull );
#define DISPATCH_BOOL(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return false;

LIB_EXPORT rc_t CC KColumnAddRef ( const KColumn *self )
{
    DISPATCH( addRef( self ) );
}
LIB_EXPORT rc_t CC KColumnRelease ( const KColumn *self )
{
    DISPATCH( release( self ) );
}
KDB_EXTERN bool CC KColumnLocked ( const KColumn *self )
{
    DISPATCH_BOOL( locked( self ) );
}
KDB_EXTERN rc_t CC KColumnVersion ( const KColumn *self, uint32_t *version )
{
    DISPATCH( version( self, version ) );
}
KDB_EXTERN rc_t CC KColumnByteOrder ( const KColumn *self, bool *reversed )
{
    DISPATCH( byteOrder( self, reversed ) );
}
KDB_EXTERN rc_t CC KColumnIdRange ( const KColumn *self, int64_t *first, uint64_t *count )
{
    DISPATCH( idRange( self, first, count ) );
}
KDB_EXTERN rc_t CC KColumnFindFirstRowId ( const KColumn * self, int64_t * found, int64_t start )
{
    DISPATCH( findFirstRowId( self, found, start ) );
}
KDB_EXTERN rc_t CC KColumnOpenManagerRead ( const KColumn *self, struct KDBManager const **mgr )
{
    DISPATCH( openManagerRead( self, mgr ) );
}
KDB_EXTERN rc_t CC KColumnOpenParentRead ( const KColumn *self, struct KTable const **tbl )
{
    DISPATCH( openParentRead( self, tbl ) );
}
LIB_EXPORT rc_t CC KColumnOpenMetadataRead ( const KColumn *self, const KMetadata **metap )
{
    DISPATCH( openMetadataRead( self, metap ) );
}
LIB_EXPORT rc_t CC KColumnOpenBlobRead ( const KColumn *self, const KColumnBlob **blobp, int64_t id )
{
    DISPATCH( openBlobRead( self, blobp, id ) );
}

//TODO: write-side only

// KDB_EXTERN rc_t CC KColumnReindex ( KColumn *self )
// {
//     DISPATCH( reindex( self ) );
// }

// KDB_EXTERN rc_t CC KColumnCommitFreq ( KColumn *self, uint32_t *freq )
// {
//     DISPATCH( commitFreq( self, freq ) );
// }

// KDB_EXTERN rc_t CC KColumnSetCommitFreq ( KColumn *self, uint32_t freq )
// {
//     DISPATCH( setCommitFreq( self, freq ) );
// }

// KDB_EXTERN rc_t CC KColumnOpenManagerUpdate ( KColumn *self, struct KDBManager **mgr )
// {
//     DISPATCH( openManagerUpdate( self, mgr ) );
// }

// KDB_EXTERN rc_t CC KColumnOpenParentUpdate ( KColumn *self, struct KTable **tbl )
// {
//     DISPATCH( openParentUpdate( self, tbl ) );
// }
