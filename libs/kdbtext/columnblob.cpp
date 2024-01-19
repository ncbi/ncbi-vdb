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

#include "columnblob.hpp"

#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/data-buffer.h>

using namespace KDBText;
using namespace std;

/*--------------------------------------------------------------------------
 * KTextColumnBlob
 *  one or more rows of column data
 */

static rc_t KTextColumnBlobWhack ( KColumnBlob *self );
static rc_t CC KTextColumnBlobRead ( const KColumnBlob *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
static rc_t CC KTextColumnBlobReadAll ( const KColumnBlob * self, KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size );
static rc_t CC KTextColumnBlobValidate ( const KColumnBlob *self );
static rc_t CC KTextColumnBlobValidateBuffer ( const KColumnBlob * self, const KDataBuffer * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size );
static rc_t CC KTextColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count );

static KColumnBlob_vt KTextColumnBlob_vt =
{
    /* Public API */
    KTextColumnBlobWhack,
    KColumnBlobBaseAddRef,
    KColumnBlobBaseRelease,
    KTextColumnBlobRead,
    KTextColumnBlobReadAll,
    KTextColumnBlobValidate,
    KTextColumnBlobValidateBuffer,
    KTextColumnBlobIdRange
};

#define CAST() assert( bself->vt == &KTextColumnBlob_vt ); ColumnBlob * self = (ColumnBlob *)bself

ColumnBlob::ColumnBlob( const void * data, size_t size, const Column * col, int64_t id, uint64_t count )
:   m_data ( data ),
    m_size( size ),
    m_parent( col ),
    m_firstRow( id ),
    m_count( count )
{
    dad . vt = & KTextColumnBlob_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::ColumnBlob", "ctor", "db" );
}

ColumnBlob::~ColumnBlob()
{
    KRefcountWhack ( & dad . refcount, "KDBText::ColumnBlob" );
}

// API

static
rc_t
KTextColumnBlobWhack ( KColumnBlob *bself )
{
    CAST();

    delete reinterpret_cast<ColumnBlob*>( self );
    return 0;
}

static
rc_t CC
KTextColumnBlobRead ( const KColumnBlob *bself, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining )
{
    CAST();

    rc_t rc = 0;
    if ( buffer == NULL )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcBuffer, rcNull );
    }
    else if ( num_read == nullptr )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    }
    else if ( offset > self -> getSize() )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcExcessive );
    }
    else
    {
        size_t toRead = self -> getSize() - offset;
        if ( toRead > bsize )
        {
            * remaining = toRead - bsize;
            toRead -= *remaining;
        }
        else
        {
            * remaining = 0;
        }
        memcpy( buffer, (const char*)(self -> getData()) + offset, toRead );
        *num_read = toRead;
    }

    return rc;
}

static
rc_t CC
KTextColumnBlobReadAll ( const KColumnBlob * bself, KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size )
{
    CAST();

    if ( opt_cs_data != NULL )
    {
        memset ( opt_cs_data, 0, cs_data_size ); // we do not populate checksum here
    }

    rc_t rc = 0;
    if ( buffer == NULL )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    }
    else
    {
        rc = KDataBufferMakeBytes ( buffer, self->getSize() );
        if ( rc == 0 && self->getSize() > 0 )
        {
            memmove( buffer -> base, self -> getData(), self->getSize() );
        }
    }

    return rc;
}

static
rc_t CC
KTextColumnBlobValidate ( const KColumnBlob *self )
{
    return 0;
}

static
rc_t CC
KTextColumnBlobValidateBuffer ( const KColumnBlob * bself, const KDataBuffer * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size )
{
    CAST();

    if ( buffer == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    }
    if ( cs_data == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    }

    // check the buffer's size
    size_t bsize = KDataBufferBytes ( buffer );
    if ( bsize < self -> getSize() )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcData, rcInsufficient );
    }
    if ( bsize > self -> getSize() )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcData, rcExcessive );
    }

    return 0;
}

static
rc_t CC
KTextColumnBlobIdRange ( const KColumnBlob * bself, int64_t *first, uint32_t *count )
{
    CAST();

    if ( first == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    }
    if ( count == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcAccessing, rcParam, rcNull );
    }

    *first = self->getIdRange().first;
    *count = self->getIdRange().second;
    return 0;
}
