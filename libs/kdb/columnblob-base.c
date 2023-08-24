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

#define KColumnBlob KColumnBlobBase
#include "columnblob-base.h"

#include <kdb/column.h>

#include <kdb/extern.h>

#include <klib/rc.h>

void KColumnBlobBaseInit( KCOLUMNBLOB_IMPL *self, const KColumnBlobBase_vt * vt )
{
    atomic32_set ( & self -> refcount, 1 );
    self -> vt = vt;
}

rc_t CC KColumnBlobBaseWhack ( KColumnBlob *self )
{
    KRefcountWhack ( & self -> refcount, "KColumnBlob" );
    free ( self );
    return 0;
}

/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
rc_t CC KColumnBlobBaseAddRef ( const KColumnBlob *cself )
{
    KColumnBlob *self = ( KColumnBlob* ) cself;
    atomic32_inc ( & self -> refcount );
    return 0;
}

rc_t CC KColumnBlobBaseRelease ( const KColumnBlob *cself )
{
    KColumnBlob *self = ( KColumnBlob* ) cself;
    if ( atomic32_dec_and_test ( & self -> refcount ) )
        return self -> vt -> whack ( self );
    return 0;
}

/******************* dispatch functions ***********************/
#define DISPATCH(call)  \
    if ( self != NULL && self -> vt != NULL )   \
        return self -> vt -> call;              \
    else                                        \
        return RC ( rcVDB, rcCursor, rcAccessing, rcSelf, rcNull );

LIB_EXPORT rc_t CC KColumnBlobAddRef ( const KColumnBlob *self )
{
    DISPATCH( addRef( self ) );
}
LIB_EXPORT rc_t CC KColumnBlobRelease ( const KColumnBlob *self )
{
    DISPATCH( release( self ) );
}
LIB_EXPORT rc_t CC KColumnBlobRead ( const KColumnBlob *self,
    size_t offset, void *buffer, size_t bsize,
    size_t *num_read, size_t *remaining )
{
    DISPATCH( read( self, offset, buffer, bsize, num_read, remaining ) );
}
LIB_EXPORT rc_t CC KColumnBlobReadAll ( const KColumnBlob * self, struct KDataBuffer * buffer,
    KColumnBlobCSData * opt_cs_data, size_t cs_data_size )
{
    DISPATCH( readAll( self, buffer, opt_cs_data, cs_data_size ) );
}
LIB_EXPORT rc_t CC KColumnBlobValidate ( const KColumnBlob *self )
{
    DISPATCH( validate( self ) );
}
LIB_EXPORT rc_t CC KColumnBlobValidateBuffer ( const KColumnBlob * self,
    struct KDataBuffer const * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size )
{
    DISPATCH( validateBuffer( self, buffer, cs_data, cs_data_size ) );
}
LIB_EXPORT rc_t CC KColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count )
{
    DISPATCH( idRange( self, first, count ) );
}
