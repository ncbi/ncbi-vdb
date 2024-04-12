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

#include <kdb/page-map.h>

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

ColumnBlob::ColumnBlob( int64_t startId )
: m_startId( startId )
{
    dad . vt = & KTextColumnBlob_vt;
    KRefcountInit ( & dad . refcount, 1, "KDBText::ColumnBlob", "ctor", "db" );

    KDataBufferMake ( & m_data, 8, 0 );
}

ColumnBlob::~ColumnBlob()
{
    //TODO: whack m_rows, m_data, m_pm
    KRefcountWhack ( & dad . refcount, "KDBText::ColumnBlob" );
}

void
ColumnBlob::addRef( const ColumnBlob * b )
{
    if ( b != nullptr )
    {
        KColumnBlobAddRef( (const KColumnBlob*) b );
    }
}

void
ColumnBlob::release( const ColumnBlob * b )
{
    if ( b != nullptr )
    {
        KColumnBlobRelease( (const KColumnBlob*) b );
    }
}

rc_t
ColumnBlob::appendRow( const void * data, size_t size, uint32_t count )
{
    KDataBuffer b;
    rc_t rc = KDataBufferMake ( &b, 8, size );
    if ( rc != 0 )
    {
        return rc;
    }
    m_rows.push_back( b );
    m_count += count;
    bool same_data = false; //TODO: calculate for real
    return PageMapAppendRows( m_pm, size, count, same_data );
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

// copied from interfaces/vdb/xform.h
typedef uint8_t VByteOrder;
enum
{
    vboNone,
    vboNative,
    vboLittleEndian,
    vboBigEndian
};

static
rc_t CC
KTextColumnBlobRead ( const KColumnBlob *bself, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining )
{
    CAST();

    rc_t rc = 0;
    if ( buffer == nullptr && bsize != 0 )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcBuffer, rcNull );
    }
    else if ( num_read == nullptr )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    }
    else if ( offset > self -> getDataSize() )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcExcessive );
    }
    else
    {
        size_t toRead = self -> getDataSize() - offset;
        if ( bsize == 0 )
        {
            * remaining = toRead;
            * num_read = 0;
        }
        else
        {
            if ( toRead > bsize )
            {
                * remaining = toRead - bsize;
                toRead -= *remaining;
            }
            else
            {
                * remaining = 0;
            }

            if ( toRead > 0 )
            {
                memmove( buffer, (const char*)(self -> getData() . base) + offset, toRead );
            }
            *num_read = toRead;
        }
    }

    return rc;
}

static
rc_t CC
KTextColumnBlobReadAll ( const KColumnBlob * bself, KDataBuffer * dbuffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size )
{
    CAST();

    if ( opt_cs_data != NULL )
    {
        memset ( opt_cs_data, 0, cs_data_size ); // we do not populate checksum here
    }

    rc_t rc = 0;
    if ( dbuffer == NULL )
    {
        rc = SILENT_RC ( rcDB, rcBlob, rcReading, rcParam, rcNull );
    }
    else
    {
        rc = KDataBufferMakeBytes ( dbuffer, self->getDataSize() );
        if ( rc == 0 && self->getDataSize() > 0 )
        {
            memmove( dbuffer -> base, self -> getData() . base, self->getDataSize() );
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
KTextColumnBlobValidateBuffer ( const KColumnBlob * bself, const KDataBuffer * dbuffer, const KColumnBlobCSData * cs_data, size_t cs_data_size )
{
    CAST();

    if ( dbuffer == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    }
    if ( cs_data == NULL )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcParam, rcNull );
    }

    // check the buffer's size
    size_t bsize = KDataBufferBytes ( dbuffer );
    if ( bsize < self -> getDataSize() )
    {
        return SILENT_RC ( rcDB, rcBlob, rcValidating, rcData, rcInsufficient );
    }
    if ( bsize > self -> getDataSize() )
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
