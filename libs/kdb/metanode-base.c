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

#define KMDataNode KMDataNodeBase
#include "metanode-base.h"

rc_t KMDataNodeBaseWhack ( KMDataNodeBase *self )
{
    KRefcountWhack ( & self -> refcount, "KMDataNode" );
    free ( self );
    return 0;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KMDataNodeBaseAddRef ( const KMDataNodeBase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KMDataNode" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcMetadata, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Attach
 * Sever
 */
KMDataNodeBase *KMDataNodeAttach ( const KMDataNodeBase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KMDataNode" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KMDataNodeBase* ) self;
}

rc_t KMDataNodeSever ( const KMDataNodeBase *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KMDataNode" ) )
        {
        case krefWhack:
            return self -> vt -> whack ( (KMDataNodeBase *)self );
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
        return RC ( rcVDB, rcMgr, rcAccessing, rcSelf, rcNull );
#define DISPATCH_BOOL(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return false;

LIB_EXPORT rc_t KMDataNodeWhack ( KMDataNode *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( whack( self ) );
}

LIB_EXPORT rc_t CC KMDataNodeAddRef ( const KMDataNode *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( addRef( self ) );
}
LIB_EXPORT rc_t CC KMDataNodeRelease ( const KMDataNode *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( release( self ) );
}
LIB_EXPORT rc_t CC KMDataNodeByteOrder ( const KMDataNode *self, bool *reversed )
{
    DISPATCH( byteOrder( self, reversed ) );
}
LIB_EXPORT rc_t CC KMDataNodeRead ( const KMDataNode *self,size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining )
{
    DISPATCH( read( self, offset, buffer, bsize, num_read, remaining ) );
}
LIB_EXPORT rc_t CC KMDataNodeVOpenNodeRead ( const KMDataNode *self, const KMDataNode **node, const char *path, va_list args )
{
    DISPATCH( openNodeRead( self, node, path, args ) );
}
LIB_EXPORT rc_t CC KMDataNodeOpenNodeRead ( const KMDataNode *self, const KMDataNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KMDataNodeVOpenNodeRead ( self, node, path, args );
    va_end ( args );

    return rc;
}
LIB_EXPORT rc_t CC KMDataNodeReadB8 ( const KMDataNode *self, void *b8 )
{
    DISPATCH( readB8( self, b8 ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadB16 ( const KMDataNode *self, void *b16 )
{
    DISPATCH( readB16( self, b16 ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadB32 ( const KMDataNode *self, void *b32 )
{
    DISPATCH( readB32( self, b32 ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadB64 ( const KMDataNode *self, void *b64 )
{
    DISPATCH( readB64( self, b64 ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadB128 ( const KMDataNode *self, void *b128 )
{
    DISPATCH( readB128( self, b128 ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAsI16 ( const KMDataNode *self, int16_t *i )
{
    DISPATCH( readAsI16( self, i ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAsU16 ( const KMDataNode *self, uint16_t *u )
{
    DISPATCH( readAsU16( self, u ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAsI32 ( const KMDataNode *self, int32_t *i )
{
    DISPATCH( readAsI32( self, i ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAsU32 ( const KMDataNode *self, uint32_t *u )
{
    DISPATCH( readAsU32( self, u ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAsI64 ( const KMDataNode *self, int64_t *i )
{
    DISPATCH( readAsI64( self, i ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAsU64 ( const KMDataNode *self, uint64_t *u )
{
    DISPATCH( readAsU64( self, u ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAsF64 ( const KMDataNode *self, double *f )
{
    DISPATCH( readAsF64( self, f ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadCString ( const KMDataNode *self, char *buffer, size_t bsize, size_t *size )
{
    DISPATCH( readCString( self, buffer, bsize, size ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAttr ( const KMDataNode *self, const char *name, char *buffer, size_t bsize, size_t *size )
{
    DISPATCH( readAttr( self, name, buffer, bsize, size ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsI16 ( const KMDataNode *self, const char *attr, int16_t *i )
{
    DISPATCH( readAttrAsI16( self, attr, i ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsU16 ( const KMDataNode *self, const char *attr, uint16_t *u )
{
    DISPATCH( readAttrAsU16( self, attr, u ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsI32 ( const KMDataNode *self, const char *attr, int32_t *i )
{
    DISPATCH( readAttrAsI32( self, attr, i ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsU32 ( const KMDataNode *self, const char *attr, uint32_t *u )
{
    DISPATCH( readAttrAsU32( self, attr, u ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsI64 ( const KMDataNode *self, const char *attr, int64_t *i )
{
    DISPATCH( readAttrAsI64( self, attr, i ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsU64 ( const KMDataNode *self, const char *attr, uint64_t *u )
{
    DISPATCH( readAttrAsU64( self, attr, u ) );
}
LIB_EXPORT rc_t CC KMDataNodeReadAttrAsF64 ( const KMDataNode *self, const char *attr, double *f )
{
    DISPATCH( readAttrAsF64( self, attr, f ) );
}
LIB_EXPORT rc_t CC KMDataNodeCompare( const KMDataNode *self, const KMDataNode *other, bool *equal )
{
    DISPATCH( compare( self, other, equal ) );
}
