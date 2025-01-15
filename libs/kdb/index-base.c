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

#include "index-base.h"

#include <klib/rc.h>

rc_t KIndexBaseWhack ( KIndex *self )
{
    KRefcountWhack ( & self -> refcount, "KIndex" );
    free ( self );
    return 0;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
rc_t CC KIndexBaseAddRef ( const KIndex *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KIndex" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcMgr, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

rc_t CC KIndexBaseRelease ( const KIndex *self )
{
    switch ( KRefcountDrop ( & self -> refcount, "KIndex" ) )
    {
    case krefWhack:
        return self -> vt -> whack ( ( KIndex* ) self );
    case krefNegative:
        return RC ( rcDB, rcMgr, rcReleasing, rcRange, rcExcessive );
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
#define DISPATCH_VOID(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        self -> vt -> call;

LIB_EXPORT rc_t CC KIndexWhack ( KIndex *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( whack( self ) );
}
LIB_EXPORT rc_t CC KIndexAddRef ( const KIndex *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( addRef( self ) );
}
LIB_EXPORT rc_t CC KIndexRelease ( const KIndex *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( release( self ) );
}
LIB_EXPORT bool CC KIndexLocked ( const KIndex *self )
{
    DISPATCH_BOOL( locked( self ) );
}
LIB_EXPORT rc_t CC KIndexVersion ( const KIndex *self, uint32_t *version )
{
    DISPATCH( version( self, version ) );
}
LIB_EXPORT rc_t CC KIndexType ( const KIndex *self, KIdxType *type )
{
    DISPATCH( type( self, type ) );
}
LIB_EXPORT rc_t CC KIndexConsistencyCheck ( const KIndex *self, uint32_t level,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes )
{
    DISPATCH( consistencyCheck( self, level, start_id, id_range, num_keys, num_rows, num_holes ) );
}
LIB_EXPORT rc_t CC KIndexFindText ( const KIndex *self, const char *key, int64_t *start_id, uint64_t *id_count,
    int ( CC * custom_cmp ) ( const void *item, struct PBSTNode const *n, void *data ),
    void *data )
{
    DISPATCH( findText( self, key, start_id, id_count, custom_cmp, data ) );
}
LIB_EXPORT rc_t CC KIndexFindAllText ( const KIndex *self, const char *key,
    rc_t ( CC * f ) ( int64_t id, uint64_t id_count, void *data ), void *data )
{
    DISPATCH( findAllText( self, key, f, data ) );
}
LIB_EXPORT rc_t CC KIndexProjectText ( const KIndex *self,
    int64_t id, int64_t *start_id, uint64_t *id_count,
    char *key, size_t kmax, size_t *actsize )
{
    DISPATCH( projectText( self, id, start_id, id_count, key, kmax, actsize ) );
}
LIB_EXPORT rc_t CC KIndexProjectAllText ( const KIndex *self, int64_t id,
    rc_t ( CC * f ) ( int64_t start_id, uint64_t id_count, const char *key, void *data ),
    void *data )
{
    DISPATCH( projectAllText( self, id, f, data ) );
}
LIB_EXPORT rc_t CC KIndexFindU64( const KIndex* self, uint64_t offset, uint64_t* key, uint64_t* key_size, int64_t* id, uint64_t* id_qty )
{
    DISPATCH( findU64( self, offset, key, key_size, id, id_qty ) );
}
LIB_EXPORT rc_t CC KIndexFindAllU64( const KIndex* self, uint64_t offset,
    rc_t ( CC * f )(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data ), void* data)
{
    DISPATCH( findAllU64( self, offset, f, data ) );
}
LIB_EXPORT void CC KIndexSetMaxRowId ( const KIndex *self, int64_t max_row_id )
{
    DISPATCH_VOID( setMaxRowId( self, max_row_id ) );
}