/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICEKColumnBlob
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

#include "ColumnBlob.hpp"

#include <kdb/column.h>

#include <klib/rc.h>

KColumnBlobBase::~KColumnBlobBase() {}

/******************* dispatch functions ***********************/
#define DISPATCH(call)  \
    if ( self != NULL )   \
        return reinterpret_cast<KColumnBlobBase*>(const_cast<KColumnBlob *>(self)) -> call; \
    else                                        \
        return RC ( rcDB, rcCursor, rcAccessing, rcSelf, rcNull );

#define CONST_DISPATCH(call)  \
    if ( self != NULL )   \
        return reinterpret_cast<const KColumnBlobBase*>(self) -> call; \
    else                                        \
        return RC ( rcDB, rcCursor, rcAccessing, rcSelf, rcNull );

LIB_EXPORT rc_t CC KColumnBlobAddRef ( const KColumnBlob *self )
{
    DISPATCH( addRef() );
}
LIB_EXPORT rc_t CC KColumnBlobRelease ( const KColumnBlob *self )
{
    DISPATCH( release() );
}
LIB_EXPORT rc_t CC KColumnBlobRead ( const KColumnBlob *self,
    size_t offset, void *buffer, size_t bsize,
    size_t *num_read, size_t *remaining )
{
    CONST_DISPATCH( read( offset, buffer, bsize, num_read, remaining ) );
}
LIB_EXPORT rc_t CC KColumnBlobReadAll ( const KColumnBlob * self, struct KDataBuffer * buffer,
    KColumnBlobCSData * opt_cs_data, size_t cs_data_size )
{
    CONST_DISPATCH( readAll( buffer, opt_cs_data, cs_data_size ) );
}
LIB_EXPORT rc_t CC KColumnBlobValidate ( const KColumnBlob *self )
{
    CONST_DISPATCH( validate() );
}
LIB_EXPORT rc_t CC KColumnBlobValidateBuffer ( const KColumnBlob * self,
    struct KDataBuffer const * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size )
{
    CONST_DISPATCH( validateBuffer( buffer, cs_data, cs_data_size ) );
}
LIB_EXPORT rc_t CC KColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count )
{
    CONST_DISPATCH( idRange( first, count ) );
}
