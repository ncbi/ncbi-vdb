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

#include "meta-base.h"

#include <kdb/extern.h>

#include <klib/rc.h>

rc_t KMetadataBaseWhack ( KMetadata *self )
{
    KRefcountWhack ( & self -> refcount, "KMetadata" );
    free ( self );
    return 0;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
rc_t CC KMetadataBaseAddRef ( const KMetadata *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KMetadata" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcMgr, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

rc_t CC KMetadataBaseRelease ( const KMetadata *self )
{
    switch ( KRefcountDrop ( & self -> refcount, "KMetadata" ) )
    {
    case krefWhack:
        return self -> vt -> whack ( ( KMetadata* ) self );
    case krefNegative:
        return RC ( rcDB, rcMgr, rcReleasing, rcRange, rcExcessive );
    }
    return 0;
}

/* Attach
 * Sever
 */
KMetadata *KMetadataAttach ( const KMetadata *self )
{

    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "KMetadata" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( KMetadata* ) self;
}

rc_t KMetadataSever ( const KMetadata *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "KMetadata" ) )
        {
        case krefWhack:
            return self -> vt -> whack ( ( KMetadata* ) self );
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

LIB_EXPORT rc_t CC KMetadataAddRef ( const KMetadata *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( addRef( self ) );
}
LIB_EXPORT rc_t CC KMetadataRelease ( const KMetadata *self )
{
    if ( self == NULL ) return 0;
    DISPATCH( release( self ) );
}
LIB_EXPORT rc_t CC KMetadataVersion ( const KMetadata *self, uint32_t *version )
{
    DISPATCH( version( self, version ) );
}
LIB_EXPORT rc_t CC KMetadataByteOrder ( const KMetadata *self, bool *reversed )
{
    DISPATCH( byteOrder( self, reversed ) );
}
LIB_EXPORT rc_t CC KMetadataRevision ( const KMetadata *self, uint32_t *revision )
{
    DISPATCH( revision( self, revision ) );
}
LIB_EXPORT rc_t CC KMetadataMaxRevision ( const KMetadata *self, uint32_t *revision )
{
    DISPATCH( maxRevision( self, revision ) );
}
LIB_EXPORT rc_t CC KMetadataOpenRevision ( const KMetadata *self,
    const KMetadata **metap, uint32_t revision )
{
    DISPATCH( openRevision( self, metap, revision ) );
}
LIB_EXPORT rc_t CC KMetadataGetSequence ( const KMetadata *self, const char *seq, int64_t *val )
{
    DISPATCH( getSequence( self, seq, val ) );
}
LIB_EXPORT rc_t CC KMetadataVOpenNodeRead ( const KMetadata *self, const KMDataNode **node, const char *path, va_list args )
{
    DISPATCH( vOpenNodeRead( self, node, path, args ) );
}
LIB_EXPORT rc_t CC KMetadataOpenNodeRead ( const KMetadata *self, const KMDataNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KMetadataVOpenNodeRead ( self, node, path, args );
    va_end ( args );

    return rc;
}




