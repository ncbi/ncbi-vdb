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

#include <simple/extern.h>

typedef struct VDBSpotRead VDBSpotRead;
#define SRASPOT VDBSpotRead

#include "vdb-spot-priv.h"

#include "sra-exception.h"
#include "sra-spot-priv.h"
#include "sra-string.h"
#include "sra-mem.h"
#include "sra-memmgr.h"

#include <insdc/insdc.h>
#include <vdb/cursor.h>
#include <vdb/blob.h>
#include <vdb/vdb-priv.h>
#include <klib/rc.h>

#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VBlob
 */
static
bool VBlobIsSingleRow ( const VBlob *self, SRAException *x, const char *col_name )
{
    int64_t first;
    uint64_t count;
    rc_t rc = VBlobIdRange ( self, & first, & count );
    if ( rc != 0 )
        SRAExceptionError ( x, 0, rc, "failed to obtain %s blob row range", col_name );
    else if ( count == 1 )
        return true;
    else
    {
        const void *base1;
        uint32_t elem_bits, boff, row_len;
        rc = VBlobCellData ( self, first, & elem_bits, & base1, & boff, & row_len );
        if ( rc == 0 )
        {
            const void *base2;
            rc = VBlobCellData ( self, first + count - 1, & elem_bits, & base2, & boff, & row_len );
            if ( rc == 0 )
                return base1 == base2;
        }

        SRAExceptionError ( x, 0, rc, "failed to obtain %s blob row data", col_name );
    }

    return false;
}


/*--------------------------------------------------------------------------
 * VBlobRow
 */
typedef struct VBlobRow VBlobRow;
struct VBlobRow
{
    int64_t end;
    const VBlob *blob;
    const void *base;
    uint32_t elem_bits;
    uint32_t row_len;
};

#if 0
static
void VBlobRowWhack ( VBlobRow *self )
{
    VBlobRelease ( self -> blob );
    memset ( self, 0, sizeof * self );
}
#endif

static
bool VBlobRowInit ( VBlobRow *self, SRAException *x, const VCursor *curs,
    int64_t row_id, uint32_t col_idx, const char *col_name )
{
    uint64_t count;
    rc_t rc = VCursorGetBlobDirect ( curs, & self -> blob, row_id, col_idx );
    if ( rc != 0 )
    {
        SRAExceptionError ( x, 0, rc, "failed to retrieve %s blob", col_name );
        return false;
    }

    rc = VBlobIdRange ( self -> blob, & self -> end, & count );
    if ( rc != 0 )
    {
        VBlobRelease ( self -> blob );
        SRAExceptionError ( x, 0, rc, "failed to retrieve %s blob range", col_name );
        return false;
    }

    self -> end += count;

    rc = VBlobCellData ( self -> blob, row_id, & self -> elem_bits, & self -> base, NULL, & self -> row_len );
    if ( rc == 0 )
        return true;

    VBlobRelease ( self -> blob );
    SRAExceptionError ( x, 0, rc, "failed to retrieve %s row", col_name );
    return false;
}

static
bool VBlobRowIsStatic ( const VBlobRow *self, SRAException *x, int64_t startRow, uint64_t rowCount, const char *col_name )
{
    if ( startRow + rowCount == self -> end )
        return VBlobIsSingleRow ( self -> blob, x, col_name );

    return false;
}

static
bool VBlobRowPrepare ( const VBlobRow *self, SRAException *x, VBlobRow *next, const char *col_name )
{
    rc_t rc = VBlobAddRef ( self -> blob );
    if ( rc != 0 )
    {
        SRAExceptionError ( x, 0, rc, "failed to attach to %s blob", col_name );
        return false;
    }

    next -> end = self -> end;
    next -> blob = self -> blob;

    return true;
}

static
bool VBlobRowClone ( const VBlobRow *self, SRAException *x, VBlobRow *clone, const char *col_name )
{
    if ( ! VBlobRowPrepare ( self, x, clone, col_name ) )
        return false;

    clone -> base = self -> base;
    clone -> elem_bits = self -> elem_bits;
    clone -> row_len = self -> row_len;

    return true;
}

static
bool VBlobRowNext ( VBlobRow *self, SRAException *x, const VCursor *curs,
    int64_t row_id, uint32_t col_idx, const char *col_name )
{
    rc_t rc;

    if ( row_id >= self -> end )
    {
        int64_t start;
        uint64_t count;
        const VBlob *blob;

        rc = VCursorGetBlobDirect ( curs, & blob, row_id, col_idx );
        if ( rc != 0 )
        {
            SRAExceptionError ( x, 0, rc, "failed to retrieve %s blob", col_name );
            return false;
        }

        rc = VBlobIdRange ( blob, & start, & count );
        if ( rc != 0 )
        {
            VBlobRelease ( blob );
            SRAExceptionError ( x, 0, rc, "failed to retrieve %s blob range", col_name );
            return false;
        }

        assert ( start <= row_id );
        assert ( row_id < start + count );

        VBlobRelease ( self -> blob );
        self -> end = start + count;
        self -> blob = blob;
    }

    rc = VBlobCellData ( self -> blob, row_id, & self -> elem_bits, & self -> base, NULL, & self -> row_len );
    if ( rc != 0 )
    {
        SRAExceptionError ( x, 0, rc, "failed to retrieve %s row", col_name );
        return false;
    }

    return true;
}


/*--------------------------------------------------------------------------
 * VDBSpotRead
 */
struct VDBSpotRead
{
    SRASequence dad;

    /* standard spot row access */
    int64_t start;
    uint64_t count;
    uint64_t row;

    /* iterator information for individual reads */
    uint64_t read_cnt;
    uint64_t read_num;

    /* READ_LEN and READ_TYPE blobs */
    VBlobRow read_len;
    VBlobRow read_type;

    /* cursor and column map */
    const VCursor *curs;
    uint32_t col_idx [ num_columns ];

    /* individual read index within spot */
    uint32_t read_idx;
    uint32_t read_off;

    bool accessed;
};

static
rc_t VDBSpotReadData ( const VDBSpotRead *self, uint32_t cidx,
    uint32_t *elem_bits, const void **base, uint32_t *row_len )
{
    uint32_t boff;
    int64_t row_id = self -> start + self -> row;
    rc_t rc = VCursorCellDataDirect ( self -> curs, row_id,
        self -> col_idx [ cidx ], elem_bits, base, & boff, row_len );
    if ( rc == 0 )
    {
        /* there are no bit-aligned data types */
        assert ( boff == 0 );

        /* the test avoids dirtying the cache */
        if ( ! self -> accessed )
            ( ( VDBSpotRead* ) self ) -> accessed = true;
    }
    return rc;
}

static
INSDC_coord_len VDBSpotReadLen ( const VDBSpotRead *self )
{
    /* READ_LEN has been previously initialized */
    const INSDC_coord_len *read_len = self -> read_len . base;
    assert ( read_len != NULL );
    assert ( self -> read_len . elem_bits == sizeof * read_len * 8 );
    assert ( self -> read_idx < self -> read_len . row_len );

    /* return the length of this read */
    return read_len [ self -> read_idx ];
}

static
INSDC_read_type VDBSpotReadType ( const VDBSpotRead *self )
{
    /* READ_TYPE has been previously initialized */
    const INSDC_read_type *read_type = self -> read_type . base;
    assert ( read_type != NULL );
    assert ( self -> read_type . elem_bits == sizeof * read_type * 8 );
    assert ( self -> read_idx < self -> read_type . row_len );

    /* return the type of this read */
    return read_type [ self -> read_idx ];
}


/*--------------------------------------------------------------------------
 * SRASequence
 */

static
void CC VDBSpotReadWhack ( VDBSpotRead *self )
{
    VBlobRelease ( self -> read_len . blob );
    VBlobRelease ( self -> read_type . blob );
    VCursorRelease ( self -> curs );
    SRAMemMgrFree ( g_mmgr, self, sizeof * self );
}

static
VDBSpotRead *VDBSpotReadDup ( const VDBSpotRead *self, SRAException *x );

static
SRAString CC VDBSpotReadName ( const VDBSpotRead *self, SRAException *x )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotReadData ( self, cidx_NAME, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        assert ( elem_bits == 8 );
        return SRAStringMake ( NULL, base, row_len );
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access spot name" );
    return SRAStringMake ( NULL, "", 0 );
}

static
bool CC VDBSpotReadIsTechnical ( const VDBSpotRead *self, SRAException *x )
{
    return ( VDBSpotReadType ( self ) & 1 ) == READ_TYPE_TECHNICAL;
}

static
bool CC VDBSpotReadIsReverse ( const VDBSpotRead *self, SRAException *x )
{
    return ( VDBSpotReadType ( self ) & READ_TYPE_REVERSE ) != 0;
}

static
uint64_t CC VDBSpotReadLength ( const VDBSpotRead *self, SRAException *x )
{
    return VDBSpotReadLen ( self );
}

static
SRAString CC VDBSpotReadBases ( const VDBSpotRead *self,
    SRAException *x, uint64_t offset, uint64_t *remaining )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotReadData ( self, cidx_READ, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        INSDC_coord_len read_len;
        const INSDC_dna_text *bases = base;

        if ( row_len == 0 )
            return SRAStringMake ( NULL, "", 0 );

        assert ( elem_bits == sizeof * bases * 8 );

        read_len = VDBSpotReadLen ( self );

        assert ( self -> read_off + read_len <= row_len );

        return SRAStringMake ( NULL, & bases [ self -> read_off ], read_len );
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access bases" );
    return SRAStringMake ( NULL, "", 0 );
}

static
bool CC VDBSpotReadColorSpaceNative ( const VDBSpotRead *self, SRAException *x )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotReadData ( self, cidx_CS_NATIVE, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        const bool *is_native = base;
        if ( row_len == 0 )
            return false;

        assert ( elem_bits == sizeof * is_native * 8 );

        return is_native [ 0 ];
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access property" );
    return false;
}

static
bool CC VDBSpotReadHasColorSpace ( const VDBSpotRead *self )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotReadData ( self, cidx_CSREAD, & elem_bits, & base, & row_len );
    return ( rc == 0 );
}

static
char CC VDBSpotReadColorSpaceKey ( const VDBSpotRead *self, SRAException *x )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotReadData ( self, cidx_CS_KEY, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        const INSDC_dna_text *cs_key = base;

        if ( row_len == 0 )
            return 0;

        assert ( elem_bits == sizeof * cs_key * 8 );
        assert ( self -> read_idx < row_len );

        return cs_key [ self -> read_idx ];
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access color-space key" );
    return 0;
}

static
SRAString CC VDBSpotReadColorSpace ( const VDBSpotRead *self,
    SRAException *x, uint64_t offset, uint64_t *remaining )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotReadData ( self, cidx_CSREAD, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        INSDC_coord_len read_len;
        const INSDC_color_text *colors = base;

        if ( row_len == 0 )
            return SRAStringMake ( NULL, "", 0 );

        assert ( elem_bits == sizeof * colors * 8 );

        read_len = VDBSpotReadLen ( self );

        assert ( self -> read_off + read_len <= row_len );

        return SRAStringMake ( NULL, & colors [ self -> read_off ], read_len );
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access color-space" );
    return SRAStringMake ( NULL, "", 0 );
}

static
SRAMem CC VDBSpotReadQualities ( const VDBSpotRead *self,
    SRAException *x, uint64_t offset, uint64_t *remaining )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotReadData ( self, cidx_QUALITY, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        INSDC_coord_len read_len;
        const INSDC_quality_phred *qual = base;

        if ( row_len == 0 )
            return SRAMemMake ( NULL, "", 0 );

        assert ( elem_bits == sizeof * qual * 8 );

        read_len = VDBSpotReadLen ( self );

        assert ( self -> read_off + read_len <= row_len );

        return SRAMemMake ( NULL, ( void* ) & qual [ self -> read_off ], read_len );
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access qualities" );
    return SRAMemMake ( NULL, "", 0 );
}

static
bool CC VDBSpotReadHasPrimaryAlignment ( const VDBSpotRead *self )
{
    return false;
}

static
struct SRA_Alignment* CC VDBSpotReadPrimaryAlignment ( const VDBSpotRead *self, SRAException *x )
{
    rc_t rc = RC ( rcSRA, rcCursor, rcAccessing, rcData, rcNotFound );
    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "this table is not aligned" );
    return NULL;
}

static SRASequence_vt_v1 VDBSpotRead_Sequence_vt =
{
    1, 0,

    /* BEGIN MINOR VERSION 0 */

    VDBSpotReadWhack,
    VDBSpotReadDup,
    VDBSpotReadName,
    VDBSpotReadIsTechnical,
    VDBSpotReadIsReverse,
    VDBSpotReadLength,
    VDBSpotReadBases,
    VDBSpotReadColorSpaceNative,
    VDBSpotReadHasColorSpace,
    VDBSpotReadColorSpaceKey,
    VDBSpotReadColorSpace,
    VDBSpotReadQualities,
    VDBSpotReadHasPrimaryAlignment,
    VDBSpotReadPrimaryAlignment

    /* END MINOR VERSION 0 */
};


/*--------------------------------------------------------------------------
 * SRASpotIterator
 */

/* forward */
static
VDBSpotRead* CC VDBSpotReadIteratorDup ( const VDBSpotRead *self, SRAException *x );

static
VDBSpotRead* CC VDBSpotBioReadIteratorDup ( const VDBSpotRead *self, SRAException *x );


/* Next
 *  advance to the next spot
 *  returns false if no more are available
 */
static
bool CC VDBSpotReadIteratorNext ( VDBSpotRead *self, SRAException *x )
{
    int64_t row_id;
    uint32_t read_idx;
    VBlobRow read_len_blob, read_type_blob;

    /* check the table-global read-id against total read count */
    uint64_t read_id = self -> read_num + self -> accessed;
    if ( read_id >= self -> read_cnt )
        return false;

    /* test if within a spot */
    read_idx = self -> read_idx + self -> accessed;
    if ( read_idx < self -> read_len . row_len )
    {
        const INSDC_coord_len *read_len;

        /* if this is the first access */
        if ( ! self -> accessed )
            return self -> accessed = true;

        /* update intra-spot info */
        read_len = self -> read_len . base;
        self -> read_idx = read_idx;
        self -> read_off += read_len [ read_idx - 1 ];

        /* update the read id */
        self -> read_num = read_id;

        /* go ahead */
        return true;
    }

    /* must have been accessed */
    assert ( self -> accessed );

    /* prepare VBlob guys for loop */
    if ( ! VBlobRowPrepare ( & self -> read_len, x, & read_len_blob, "READ_LEN" ) )
        return false;
    if ( ! VBlobRowPrepare ( & self -> read_type, x, & read_type_blob, "READ_TYPE" ) )
    {
        VBlobRelease ( read_len_blob . blob );
        return false;
    }

    /* find a non-empty row */
    for ( row_id = self -> row;; )
    {

        /* need to advance to next spot */
        if ( ++ row_id >= self -> count )
        {
            /* cause earlier test to fail next time */
            self -> read_cnt = self -> read_num;
            break;
        }

        /* advance the READ_LEN blob */
        if ( ! VBlobRowNext ( & read_len_blob, x, self -> curs,
            row_id, self -> col_idx [ cidx_READ_LEN ], "READ_LEN" ) )
        {
            break;
        }

        /* advance the READ_TYPE blob */
        if ( ! VBlobRowNext ( & read_type_blob, x, self -> curs,
            row_id, self -> col_idx [ cidx_READ_TYPE ], "READ_TYPE" ) )
        {
            break;
        }

        /* they must have the same row-length */
        assert ( read_len_blob . row_len == read_type_blob . row_len );

        /* they must have proper sizes */
        assert ( read_len_blob . elem_bits == sizeof ( INSDC_coord_len ) * 8 );
        assert ( read_type_blob . elem_bits == sizeof ( INSDC_read_type ) * 8 );

        /* there must be at least one read */
        if ( read_len_blob . row_len != 0 )
        {
            /* release the old blobs and substitute */
            VBlobRelease ( self -> read_len . blob );
            VBlobRelease ( self -> read_type . blob );
            self -> read_len = read_len_blob;
            self -> read_type = read_type_blob;

            /* update the intra-spot info */
            self -> read_idx = 0;
            self -> read_off = 0;

            /* update the read id */
            self -> read_num = read_id;

            /* update the row id */
            self -> row = row_id;

            return true;
        }
    }

    /* failure */
    VBlobRelease ( read_len_blob . blob );
    VBlobRelease ( read_type_blob . blob );
    return false;
}


static
bool CC VDBSpotBioReadIteratorNext ( VDBSpotRead *self, SRAException *x )
{
    int64_t row_id;
    VBlobRow read_len_blob, read_type_blob;

    /* check the table-global read-id against total read count */
    uint64_t read_id = self -> read_num + self -> accessed;
    if ( read_id >= self -> read_cnt )
        return false;

try_within_spot:

    /* test if within a spot */
    for ( self -> read_idx += self -> accessed;
          self -> read_idx < self -> read_len . row_len;
          ++ self -> read_idx )
    {
        const INSDC_coord_len *read_len = self -> read_len . base;
        const INSDC_read_type *read_type = self -> read_type . base;

        /* if this is the first access */
        if ( ! self -> accessed )
        {
            self -> accessed = true;
            if ( ( read_type [ self -> read_idx ] & 1 ) == READ_TYPE_BIOLOGICAL )
                return true;
            continue;
        }

        self -> read_off += read_len [ self -> read_idx - 1 ];

        /* having found a nice bio read */
        if ( ( read_type [ self -> read_idx ] & 1 ) == READ_TYPE_BIOLOGICAL )
        {
            /* update the read id */
            self -> read_num = read_id;

            /* go ahead */
            return true;
        }
    }

    /* must have been accessed */
    assert ( self -> accessed );

    /* prepare VBlob guys for loop */
    if ( ! VBlobRowPrepare ( & self -> read_len, x, & read_len_blob, "READ_LEN" ) )
        return false;
    if ( ! VBlobRowPrepare ( & self -> read_type, x, & read_type_blob, "READ_TYPE" ) )
    {
        VBlobRelease ( read_len_blob . blob );
        return false;
    }

    /* find a non-empty row */
    for ( row_id = self -> row;; )
    {

        /* need to advance to next spot */
        if ( ++ row_id >= self -> count )
        {
            /* cause earlier test to fail next time */
            self -> read_cnt = self -> read_num;
            break;
        }

        /* advance the READ_LEN blob */
        if ( ! VBlobRowNext ( & read_len_blob, x, self -> curs,
            row_id, self -> col_idx [ cidx_READ_LEN ], "READ_LEN" ) )
        {
            break;
        }

        /* advance the READ_TYPE blob */
        if ( ! VBlobRowNext ( & read_type_blob, x, self -> curs,
            row_id, self -> col_idx [ cidx_READ_TYPE ], "READ_TYPE" ) )
        {
            break;
        }

        /* they must have the same row-length */
        assert ( read_len_blob . row_len == read_type_blob . row_len );

        /* they must have proper sizes */
        assert ( read_len_blob . elem_bits == sizeof ( INSDC_coord_len ) * 8 );
        assert ( read_type_blob . elem_bits == sizeof ( INSDC_read_type ) * 8 );

        /* there must be at least one read */
        if ( read_len_blob . row_len != 0 )
        {
            /* release the old blobs and substitute */
            VBlobRelease ( self -> read_len . blob );
            VBlobRelease ( self -> read_type . blob );
            self -> read_len = read_len_blob;
            self -> read_type = read_type_blob;

            /* update the intra-spot info */
            self -> read_idx = 0;
            self -> read_off = 0;
            self -> accessed = false;

            goto try_within_spot;
        }
    }

    /* failure */
    VBlobRelease ( read_len_blob . blob );
    VBlobRelease ( read_type_blob . blob );
    return false;
}


static SRASequenceIterator_vt_v1 VDBSpotReadIterator_vt =
{
    1, 0,

    VDBSpotReadIteratorDup,
    VDBSpotReadIteratorNext
};


static SRASequenceIterator_vt_v1 VDBSpotBioReadIterator_vt =
{
    1, 0,

    VDBSpotBioReadIteratorDup,
    VDBSpotBioReadIteratorNext
};


static SRASequence_mvt VDBSpotRead_mvt =
{
    ( const SRASequence_vt* ) & VDBSpotRead_Sequence_vt,
    NULL
};


static SRASequence_mvt VDBSpotReadIterator_mvt =
{
    ( const SRASequence_vt* ) & VDBSpotRead_Sequence_vt,
    ( const SRASequenceIterator_vt* ) & VDBSpotReadIterator_vt
};


static SRASequence_mvt VDBSpotBioReadIterator_mvt =
{
    ( const SRASequence_vt* ) & VDBSpotRead_Sequence_vt,
    ( const SRASequenceIterator_vt* ) & VDBSpotBioReadIterator_vt
};

static
VDBSpotRead * VDBSpotReadMake ( const SRASequence_mvt *mvt, SRAException *x, const VCursor *curs )
{
    VDBSpotRead *spot = SRAMemMgrAlloc ( g_mmgr, x, sizeof * spot, true );
    if ( spot != NULL )
    {
        if ( SRASequenceInit ( & spot -> dad, x, mvt ) )
        {
            rc_t rc = VCursorAddRef ( curs );
            if ( rc == 0 )
            {
                spot -> curs = curs;
                return spot;
            }

            SRAExceptionError ( x, 0, rc, "failed to attach reference to VCursor" );
        }

        SRAMemMgrFree ( g_mmgr, spot, sizeof * spot );
    }
    return NULL;
}

static
VDBSpotRead * VDBSpotReadClone ( const VDBSpotRead *self, SRAException *x, const SRASequence_mvt *mvt )
{
    VDBSpotRead *clone = VDBSpotReadMake ( mvt, x, self -> curs );
    if ( clone != NULL )
    {
        clone -> start = self -> start;
        clone -> count = self -> count;
        clone -> row = self -> row;

        clone -> read_cnt = self -> read_cnt;
        clone -> read_num = self -> read_num;

        memcpy ( clone -> col_idx, self -> col_idx, sizeof clone -> col_idx );

        clone -> read_idx = self -> read_idx;
        clone -> read_off = self -> read_off;

        if ( VBlobRowClone ( & self -> read_len, x, & clone -> read_len, "READ_LEN" ) )
        {
            if ( VBlobRowClone ( & self -> read_type, x, & clone -> read_type, "READ_TYPE" ) )
                return clone;
        }

        VDBSpotReadWhack ( clone );
    }

    return NULL;
}

static
VDBSpotRead* CC VDBSpotReadDup ( const VDBSpotRead *self, SRAException *x )
{
    return VDBSpotReadClone ( self, x, & VDBSpotRead_mvt );
}

static
VDBSpotRead* CC VDBSpotReadIteratorDup ( const VDBSpotRead *self, SRAException *x )
{
    return VDBSpotReadClone ( self, x, & VDBSpotReadIterator_mvt );
}

static
VDBSpotRead* CC VDBSpotBioReadIteratorDup ( const VDBSpotRead *self, SRAException *x )
{
    return VDBSpotReadClone ( self, x, & VDBSpotBioReadIterator_mvt );
}

/* MakeIteratorInt
 */
static
void SRAExceptionFailAddColumn ( SRAException *self, const char *func, rc_t rc, const char *spec )
{
    SRAExceptionError ( self, 0, rc, "%s: failed to add column: '%s'", func, spec );
}

struct col_spec
{
    size_t idx;
    const char *spec;
};

static
bool VDBSpotReadAddColumns ( VDBSpotRead *self, SRAException *x, const char *func )
{
    rc_t rc;

    uint32_t i;
    static struct col_spec perm_cols [] =
    {
        { cidx_READ, "READ" },
        { cidx_CSREAD, "CSREAD" },
        { cidx_CS_KEY, "CS_KEY" },
        { cidx_CS_NATIVE, "CS_NATIVE" },
        { cidx_QUALITY, "QUALITY" },
        { cidx_READ_TYPE, "READ_TYPE" },
        { cidx_READ_LEN, "READ_LEN" }
    };
    static struct col_spec opt_cols [] =
    {
        { cidx_NAME, "NAME" }
    };

    /* add all of the permanent columns */
    for ( i = 0; i < sizeof perm_cols / sizeof perm_cols [ 0 ]; ++ i )
    {
        rc = VCursorAddColumn ( self -> curs, & self -> col_idx [ perm_cols [ i ] . idx ], "%s", perm_cols [ i ] . spec );
        if ( rc != 0 && GetRCState ( rc ) != rcExists )
        {
            SRAExceptionFailAddColumn ( x, func, rc, perm_cols [ i ] . spec );
            return false;
        }
    }

    /* allow post-open add */
    rc = VCursorPermitPostOpenAdd ( self -> curs );
    if ( rc != 0 && GetRCState ( rc ) != rcOpen )
    {
        SRAExceptionError ( x, 0, rc, "%s: failed to configure cursor", func );
        return false;
    }

    /* open the cursor */
    rc = VCursorOpen ( self -> curs );
    if ( rc != 0 && GetRCState ( rc ) != rcOpen )
    {
        SRAExceptionError ( x, 0, rc, "%s: failed to open cursor", func );
        return false;
    }

    /* all of the optional columns */
    for ( i = 0; i < sizeof opt_cols / sizeof opt_cols [ 0 ]; ++ i )
        VCursorAddColumn ( self -> curs, & self -> col_idx [ opt_cols [ i ] . idx ], "%s", opt_cols [ i ] . spec );

    return true;
}

static
uint32_t VDBSpotReadsPerSpot ( const VDBSpotRead *self, bool biological_only )
{
    uint32_t num_reads;

    if ( ! biological_only )
        num_reads = self -> read_type . row_len;
    else
    {
        uint32_t i;
        const INSDC_read_type *read_type = self -> read_type . base;
        assert ( read_type != NULL );
        for ( i = num_reads = 0; i < self -> read_type . row_len; ++ i )
        {
            if ( ( read_type [ i ] & 1 ) == READ_TYPE_BIOLOGICAL )
                ++ num_reads;
        }
    }

    return num_reads;
}

static
VDBSpotRead *VDBSpotReadMakeIteratorInt ( SRAException *x, const char *func, const VCursor *curs, int64_t row_id, bool biological_only )
{
    VDBSpotRead *it = VDBSpotReadMake ( biological_only ? & VDBSpotBioReadIterator_mvt : & VDBSpotReadIterator_mvt, x, curs );
    if ( it != NULL )
    {
        assert ( x != NULL );

        if ( VDBSpotReadAddColumns ( it, x, func ) )
        {
            rc_t rc = VCursorIdRange ( curs, 0, & it -> start, & it -> count );
            if ( rc != 0 )
                SRAExceptionError ( x, 0, rc, "%s: failed to determine cursor range", func );
            else
            {
                /* only use valid row_id */
                if ( row_id < it -> start || it -> start + it -> count <= row_id )
                    row_id = it -> start;

                /* initialize the READ_LEN and READ_TYPE blobs */
                if ( VBlobRowInit ( & it -> read_len, x, curs, row_id, it -> col_idx [ cidx_READ_LEN ], "READ_LEN" ) )
                {
                    if ( VBlobRowInit ( & it -> read_type, x, curs, row_id, it -> col_idx [ cidx_READ_TYPE ], "READ_TYPE" ) )
                    {
                        /* the READ_TYPE column is generally static
                           if so, it should have the same range as the cursor */
                        if ( VBlobRowIsStatic ( & it -> read_type, x, it -> start, it -> count, "READ_TYPE" ) )
                            it -> read_cnt = it -> count * VDBSpotReadsPerSpot ( it, biological_only );
                        else
                            /* the read count is not known */
                            it -> read_cnt = ~ ( uint64_t ) 0;
                        return it;
                    }
                }
            }
        }

        VDBSpotReadWhack ( it );
    }

    return NULL;
}

/* MakeIterator
 *  makes a spot iterator
 */
SRASequence *VDBSpotReadMakeIterator ( SRAException *x, const VCursor *curs, bool biological_only )
{
    VDBSpotRead *it = VDBSpotReadMakeIteratorInt ( x, __func__, curs, 1, biological_only );

    if ( it != NULL )
        return & it -> dad;

    return NULL;
}

/* MakeRangeIterator
 *  makes a spot iterator over a specific range
 */
SRASequence *VDBSpotReadMakeRangeIterator ( SRAException *x, const VCursor *curs,
    uint64_t startReadIdx, uint64_t readCount, bool biological_only )
{
    VDBSpotRead *it = VDBSpotReadMakeIteratorInt ( x, __func__, curs, 1, biological_only );
    if ( it != NULL )
    {
        /* once again, detect fixed number of reads per spot */
        if ( VBlobRowIsStatic ( & it -> read_type, x, it -> start, it -> count, "READ_TYPE" ) )
        {
            /* the requested startReadIdx should be within range */
            if ( startReadIdx < it -> read_cnt )
            {
                uint32_t num_reads;

                /* limit the read range */
                uint64_t end = startReadIdx + readCount;
                if ( end < it -> read_cnt )
                    it -> read_cnt = end;
                it -> read_num = startReadIdx;

                /* find the starting row */
                num_reads = VDBSpotReadsPerSpot ( it, biological_only );
                assert ( num_reads != 0 );
                it -> row = startReadIdx / num_reads;

                if ( it -> row != 0 )
                {
                    /* center READ_LEN and READ_TYPE on this row */
                    if ( VBlobRowNext ( & it -> read_len, x, curs, it -> start + it -> row, it -> col_idx [ cidx_READ_LEN ], "READ_LEN" ) )
                        VBlobRowNext ( & it -> read_type, x, curs, it -> start + it -> row, it -> col_idx [ cidx_READ_TYPE ], "READ_TYPE" );
                }

                if ( SRAExceptionOK ( x ) )
                {
                    uint32_t num_to_skip = ( uint32_t ) ( startReadIdx % num_reads );
                    for ( it -> accessed = true; num_to_skip != 0; -- num_to_skip )
                    {
                        if ( ! SRASequenceIteratorNext ( & it -> dad, x ) )
                            break;
                    }
                    it -> accessed = false;

                    if ( SRAExceptionOK ( x ) )
                        return & it -> dad;
                }
            }
            
        }
        else
        {
            for ( it -> accessed = true; startReadIdx != 0; -- startReadIdx )
            {
                if ( ! SRASequenceIteratorNext ( & it -> dad, x ) )
                    break;
            }
            it -> accessed = false;

            if ( SRAExceptionOK ( x ) )
                return & it -> dad;
        }

        VDBSpotReadWhack ( it );
    }
    return NULL;
}

/* MakeSpotIterator
 *  makes a spot-read iterator over the reads of a single spot
 */
SRASequence *VDBSpotReadMakeSpotIterator ( SRAException *x, const VCursor *curs,
    int64_t row_id, bool biological_only )
{
    VDBSpotRead *it = VDBSpotReadMakeIteratorInt ( x, __func__, curs, row_id, biological_only );
    if ( it != NULL )
    {
        if ( it -> start <= row_id && row_id < it -> start + it -> count )
        {
            it -> start = row_id;
            it -> count = 1;
            it -> read_cnt = it -> read_type . row_len;
            return & it -> dad;
        }
    }
    return NULL;
}
