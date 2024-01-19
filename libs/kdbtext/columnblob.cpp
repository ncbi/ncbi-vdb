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

using namespace KDBText;
using namespace std;

/*--------------------------------------------------------------------------
 * KTextColumnBlob
 *  one or more rows of column data
 */

static rc_t KTextColumnBlobWhack ( KColumnBlob *self );
// static rc_t CC KTextColumnBlobRead ( const KColumnBlob *self, size_t offset, void *buffer, size_t bsize, size_t *num_read, size_t *remaining );
// static rc_t CC KTextColumnBlobReadAll ( const KColumnBlob * self, KDataBuffer * buffer, KColumnBlobCSData * opt_cs_data, size_t cs_data_size );
// static rc_t CC KTextColumnBlobValidate ( const KColumnBlob *self );
// static rc_t CC KTextColumnBlobValidateBuffer ( const KColumnBlob * self, const KDataBuffer * buffer, const KColumnBlobCSData * cs_data, size_t cs_data_size );
// static rc_t CC KTextColumnBlobIdRange ( const KColumnBlob *self, int64_t *first, uint32_t *count );

static KColumnBlob_vt KTextColumnBlob_vt =
{
    /* Public API */
    KTextColumnBlobWhack,
    KColumnBlobBaseAddRef,
    KColumnBlobBaseRelease,
    // KTextColumnBlobRead,
    // KTextColumnBlobReadAll,
    // KTextColumnBlobValidate,
    // KTextColumnBlobValidateBuffer,
    // KTextColumnBlobIdRange
};

#define CAST() assert( bself->vt == &KTextColumnBlob_vt ); ColumnBlob * self = (ColumnBlob *)bself

ColumnBlob::ColumnBlob( const void * data, size_t size, const Column * col, int64_t id, uint64_t count )
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
    return KColumnBlobBaseWhack ( bself );
}
