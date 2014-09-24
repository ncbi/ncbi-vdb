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

typedef struct VDBSpot VDBSpot;
#define SRASPOT VDBSpot

#include "vdb-spot-priv.h"

#include "sra-exception.h"
#include "sra-spot-priv.h"
#include "sra-string.h"
#include "sra-mem.h"
#include "sra-memmgr.h"

#include <insdc/insdc.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>
#include <klib/rc.h>

#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VDBSpot
 */
struct VDBSpot
{
    SRASpot dad;

    int64_t start;
    uint64_t count;
    uint64_t row;

    const VCursor *curs;
    uint32_t col_idx [ num_columns ];

    bool accessed;
};


/*--------------------------------------------------------------------------
 * SRASequence
 */

static
rc_t VDBSpotCellData ( const VDBSpot *self, uint32_t cidx,
    uint32_t *elem_bits, const void **base, uint32_t *row_len )
{
    uint32_t boff;
    int64_t row_id = self -> start + self -> row;
    rc_t rc = VCursorCellDataDirect ( self -> curs, row_id,
        self -> col_idx [ cidx ], elem_bits, base, & boff, row_len );
    if ( rc == 0 )
    {
        assert ( boff == 0 );
        if ( ! self -> accessed )
            ( ( VDBSpot* ) self ) -> accessed = true;
    }
    return rc;
}

static
void CC VDBSpotWhack ( VDBSpot *self )
{
    VCursorRelease ( self -> curs );
    SRAMemMgrFree ( g_mmgr, self, sizeof * self );
}

static
VDBSpot* CC VDBSpotDup ( const VDBSpot *self, SRAException *x )
{
    rc_t rc = RC ( rcSRA, rcCursor, rcCopying, rcMessage, rcUnauthorized );
    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "SRASequence.duplicate message not supported on SRASpot" );
    return NULL;
}

static
SRAString CC VDBSpotName ( const VDBSpot *self, SRAException *x )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotCellData ( self, cidx_NAME, & elem_bits, & base, & row_len );
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
bool CC VDBSpotIsTechnical ( const VDBSpot *self, SRAException *x )
{
    rc_t rc = RC ( rcSRA, rcCursor, rcCopying, rcMessage, rcUnsupported );
    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "SRASequence.isTechnical message not supported on SRASpot" );
    return false;
}

static
bool CC VDBSpotIsReverse ( const VDBSpot *self, SRAException *x )
{
    rc_t rc = RC ( rcSRA, rcCursor, rcCopying, rcMessage, rcUnsupported );
    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "SRASequence.isReverse message not supported on SRASpot" );
    return false;
}

static
uint64_t CC VDBSpotLength ( const VDBSpot *self, SRAException *x )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotCellData ( self, cidx_READ, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        assert ( elem_bits == 8 );
        return row_len;
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access spot length" );
    return 0;
}

static
SRAString CC VDBSpotBases ( const VDBSpot *self,
    SRAException *x, uint64_t offset, uint64_t *remaining )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotCellData ( self, cidx_READ, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        assert ( elem_bits == 8 );
        return SRAStringMake ( NULL, base, row_len );
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access bases" );
    return SRAStringMake ( NULL, "", 0 );
}

static
bool CC VDBSpotColorSpaceNative ( const VDBSpot *self, SRAException *x )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotCellData ( self, cidx_CS_NATIVE, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        const bool *is_native = base;
        if ( row_len == 0 )
            return false;

        assert ( elem_bits == 8 );
        return is_native [ 0 ];
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access property" );
    return false;
}

static
bool CC VDBSpotHasColorSpace ( const VDBSpot *self )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotCellData ( self, cidx_CSREAD, & elem_bits, & base, & row_len );
    return ( rc == 0 );
}

static
char CC VDBSpotColorSpaceKey ( const VDBSpot *self, SRAException *x )
{
    rc_t rc = RC ( rcSRA, rcCursor, rcCopying, rcMessage, rcUnsupported );
    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "SRASequence.colorSpaceKey message not supported on SRASpot" );
    return 0;
}

static
SRAString CC VDBSpotColorSpace ( const VDBSpot *self,
    SRAException *x, uint64_t offset, uint64_t *remaining )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotCellData ( self, cidx_CSREAD, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        assert ( elem_bits == 8 );
        return SRAStringMake ( NULL, base, row_len );
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access color-space" );
    return SRAStringMake ( NULL, "", 0 );
}

static
SRAMem CC VDBSpotQualities ( const VDBSpot *self,
    SRAException *x, uint64_t offset, uint64_t *remaining )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotCellData ( self, cidx_QUALITY, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        assert ( elem_bits == 8 );
        return SRAMemMake ( NULL, ( void* ) base, row_len );
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access qualities" );
    return SRAMemMake ( NULL, "", 0 );
}

static
bool CC VDBSpotHasPrimaryAlignment ( const VDBSpot *self )
{
    return false;
}

static
struct SRA_Alignment* CC VDBSpotPrimaryAlignment ( const VDBSpot *self, SRAException *x )
{
    rc_t rc = RC ( rcSRA, rcCursor, rcAccessing, rcData, rcNotFound );
    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "this table is not aligned" );
    return NULL;
}

static SRASequence_vt_v1 VDBSpot_Sequence_vt =
{
    1, 0,

    /* BEGIN MINOR VERSION 0 */

    VDBSpotWhack,
    VDBSpotDup,
    VDBSpotName,
    VDBSpotIsTechnical,
    VDBSpotIsReverse,
    VDBSpotLength,
    VDBSpotBases,
    VDBSpotColorSpaceNative,
    VDBSpotHasColorSpace,
    VDBSpotColorSpaceKey,
    VDBSpotColorSpace,
    VDBSpotQualities,
    VDBSpotHasPrimaryAlignment,
    VDBSpotPrimaryAlignment

    /* END MINOR VERSION 0 */
};


/*--------------------------------------------------------------------------
 * SRASpot
 */

/* Duplicate
 *  separate from SRASequenceDuplicate
 *  may perform other operations, returns derived type
 */
static
VDBSpot* CC VDBSpotDuplicate ( const VDBSpot *self, SRAException *x );


/* Platform
 *  copy out platform name in UTF-8 character set
 */
static
SRAString CC VDBSpotPlatform ( const VDBSpot *self, SRAException *x )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotCellData ( self, cidx_PLATFORM, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        assert ( elem_bits == 8 );
        return SRAStringMake ( NULL, base, row_len );
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access platform" );
    return SRAStringMake ( NULL, "", 0 );
}

/* Group
 *  copy out spotgroup name in UTF-8 character set
 */
static
SRAString CC VDBSpotGroup ( const VDBSpot *self, SRAException *x )
{
    if ( self -> col_idx [ cidx_SPOT_GROUP ] != 0 )
    {
        const void *base;
        uint32_t elem_bits, row_len;
        rc_t rc = VDBSpotCellData ( self, cidx_SPOT_GROUP, & elem_bits, & base, & row_len );
        if ( rc == 0 )
        {
            assert ( elem_bits == 8 );
            return SRAStringMake ( NULL, base, row_len );
        }

        assert ( x != NULL );
        SRAExceptionError ( x, 0, rc, "failed to access spot group" );
    }

    return SRAStringMake ( NULL, "", 0 );
}

/* Id
 *  spot id
 */
static
int64_t CC VDBSpotId ( const VDBSpot *self, SRAException *x )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotCellData ( self, cidx_SPOT_ID, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        uint32_t id;

        assert ( elem_bits == 32 );
        memcpy ( & id, base, sizeof id );
        return ( int64_t ) id;
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access spot id" );
    return 0;
}

/* NumReads
 *  number of reads in current spot
 */
static
uint32_t CC VDBSpotNumReads ( const VDBSpot *self, SRAException *x )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotCellData ( self, cidx_READ_TYPE, & elem_bits, & base, & row_len );
    if ( rc == 0 )
        return row_len;

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access property" );
    return 0;
}

/* Reads
 *  returns the reads as a sequence iterator
 */
static
SRASequence* CC VDBSpotReads ( const VDBSpot *self, SRAException *x )
{
    return VDBSpotReadMakeSpotIterator ( x, self -> curs, self -> start + self -> row, false );
}

/* NumBioReads
 *  number of biological reads in current spot
 */
static
uint32_t CC VDBSpotNumBioReads ( const VDBSpot *self, SRAException *x )
{
    const void *base;
    uint32_t elem_bits, row_len;
    rc_t rc = VDBSpotCellData ( self, cidx_READ_TYPE, & elem_bits, & base, & row_len );
    if ( rc == 0 )
    {
        uint32_t i, num_reads;
        const INSDC_read_type *read_type = base;

        assert ( elem_bits == 8 );

        assert ( READ_TYPE_TECHNICAL == 0 );
        assert ( READ_TYPE_BIOLOGICAL == 1 );

        for ( i = num_reads = 0; i < row_len; ++ i )
            num_reads += read_type [ i ] & 1;

        return num_reads;
    }

    assert ( x != NULL );
    SRAExceptionError ( x, 0, rc, "failed to access property" );
    return 0;
}

/* BioReads
 *  returns the biological reads as a sequence iterator
 *  if "trimmed" is true, apply quality and other cutoffs
 */
static
SRASequence* CC VDBSpotBioReads ( const VDBSpot *self, SRAException *x, bool trimmed )
{
    return VDBSpotReadMakeSpotIterator ( x, self -> curs, self -> start + self -> row, true );
}

static SRASpot_vt_v1 VDBSpot_vt =
{
    1, 0,

    VDBSpotDuplicate,
    VDBSpotPlatform,
    VDBSpotGroup,
    VDBSpotId,
    VDBSpotNumReads,
    VDBSpotReads,
    VDBSpotNumBioReads,
    VDBSpotBioReads
};

/*--------------------------------------------------------------------------
 * SRASpotIterator
 */

/* forward */
static
VDBSpot* CC VDBSpotIteratorDup ( const VDBSpot *self, SRAException *x );


/* Next
 *  advance to the next spot
 *  returns false if no more are available
 */
static
bool CC VDBSpotIteratorNext ( VDBSpot *self, SRAException *x )
{
    uint64_t row = self -> row + self -> accessed;
    if ( row >= self -> count )
        return false;

    if ( ! self -> accessed )
        self -> accessed = true;
    else
        self -> row = row;

    return true;
}


static SRASequenceIterator_vt_v1 VDBSpotIterator_vt =
{
    1, 0,

    VDBSpotIteratorDup,
    VDBSpotIteratorNext
};


static SRASpot_mvt VDBSpot_mvt =
{
    {
        ( const SRASequence_vt* ) & VDBSpot_Sequence_vt,
        NULL
    },
    ( const SRASpot_vt* ) & VDBSpot_vt
};


static SRASpot_mvt VDBSpotIterator_mvt =
{
    {
        ( const SRASequence_vt* ) & VDBSpot_Sequence_vt,
        ( const SRASequenceIterator_vt* ) & VDBSpotIterator_vt
    },
    ( const SRASpot_vt* ) & VDBSpot_vt
};

static
VDBSpot * VDBSpotMake ( const SRASpot_mvt *mvt, SRAException *x, const VCursor *curs )
{
    VDBSpot *spot = SRAMemMgrAlloc ( g_mmgr, x, sizeof * spot, true );
    if ( spot != NULL )
    {
        if ( SRASpotInit ( & spot -> dad, x, mvt ) )
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
VDBSpot * VDBSpotClone ( const VDBSpot *self, SRAException *x, const SRASpot_mvt *mvt )
{
    VDBSpot *clone = VDBSpotMake ( mvt, x, self -> curs );
    if ( clone != NULL )
    {
        clone -> start = self -> start;
        clone -> count = self -> count;
        clone -> row = self -> row;
        memcpy ( clone -> col_idx, self -> col_idx, sizeof clone -> col_idx );
    }
    return clone;
}

static
VDBSpot* CC VDBSpotDuplicate ( const VDBSpot *self, SRAException *x )
{
    return VDBSpotClone ( self, x, & VDBSpot_mvt );
}

static
VDBSpot* CC VDBSpotIteratorDup ( const VDBSpot *self, SRAException *x )
{
    return VDBSpotClone ( self, x, & VDBSpotIterator_mvt );
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
bool VDBSpotAddColumns ( VDBSpot *self, SRAException *x, const char *func )
{
    rc_t rc;

    uint32_t i;
    static struct col_spec perm_cols [] =
    {
        { cidx_READ, "READ" },
        { cidx_CSREAD, "CSREAD" },
        { cidx_CS_NATIVE, "CS_NATIVE" },
        { cidx_QUALITY, "QUALITY" },
        { cidx_READ_TYPE, "READ_TYPE" },
        { cidx_READ_LEN, "READ_LEN" }
    };
    static struct col_spec opt_cols [] =
    {
        { cidx_NAME, "NAME" },
        { cidx_PLATFORM, "(ascii)PLATFORM" },
        { cidx_SPOT_ID, "SPOT_ID" },
        { cidx_SPOT_GROUP, "SPOT_GROUP" }
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
VDBSpot *VDBSpotMakeIteratorInt ( SRAException *x, const char *func, const VCursor *curs )
{
    VDBSpot *it = VDBSpotMake ( & VDBSpotIterator_mvt, x, curs );
    if ( it != NULL )
    {
        if ( VDBSpotAddColumns ( it, x, func ) )
        {
            rc_t rc = VCursorIdRange ( curs, 0, & it -> start, & it -> count );
            if ( rc == 0 )
                return it;

            assert ( x != NULL );
            SRAExceptionError ( x, 0, rc, "%s: failed to determine cursor range", func );
        }

        VDBSpotWhack ( it );
    }

    return NULL;
}

/* MakeIterator
 *  makes a spot iterator
 */
SRASpot *VDBSpotMakeIterator ( SRAException *x, const VCursor *curs )
{
    VDBSpot *it = VDBSpotMakeIteratorInt ( x, __func__, curs );
    if ( it != NULL )
        return & it -> dad;
    return NULL;
}

/* MakeRangeIterator
 *  makes a spot iterator over a specific range
 */
SRASpot *VDBSpotMakeRangeIterator ( SRAException *x, const VCursor *curs,
    uint64_t startIdx, uint64_t countRequest )
{
    VDBSpot *it = VDBSpotMakeIteratorInt ( x, __func__, curs );
    if ( it != NULL )
    {
        rc_t rc;

        /* startIdx has to be within non-empty range */
        if ( it -> start < 0 || ( uint64_t ) it -> start <= startIdx )
        {
            int64_t stop = ( int64_t ) ( it -> start + it -> count );
            if ( stop > 0 && ( uint64_t ) stop > startIdx )
            {
                /* count may be excessive */
                uint64_t end = startIdx + countRequest;
                if ( end > ( uint64_t ) stop )
                    end = stop;

                /* this is the new range */
                it -> start = startIdx;
                it -> count = end - startIdx;
                return & it -> dad;
            }
        }

        rc = RC ( rcSRA, rcCursor, rcCreating, rcRange, rcInvalid );
        SRAExceptionError ( x, 0, rc, "requested iterator range ( %ld..%ld ) out of range",
                            startIdx, startIdx + countRequest - 1 );

        VDBSpotWhack ( it );
    }
    return NULL;
}
