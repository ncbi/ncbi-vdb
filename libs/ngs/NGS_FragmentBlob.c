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

#include "NGS_FragmentBlob.h"

#include <ngs/itf/Refcount.h>

#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/xc.h>

#include <klib/rc.h>

#include <vdb/cursor.h>
#include <vdb/blob.h>
#include <../libs/vdb/blob-priv.h>
#include <../libs/vdb/page-map.h>

#include "NGS_String.h"
#include "NGS_Id.h"
#include "NGS_Cursor.h"
#include "SRA_Read.h"

struct NGS_FragmentBlob
{
    NGS_Refcount dad;

    int64_t rowId;      /* rowId of the first row in the blob (can differ from the first row of VBlob) */
    const void* data;   /* start of the first row */
    uint64_t size;      /* from the start of the first row until the end of the blob */

    const NGS_String* run;
    const NGS_Cursor* curs;
    const VBlob* blob_READ;
    const VBlob* blob_READ_LEN;
    const VBlob* blob_READ_TYPE;
};


void
NGS_FragmentBlobWhack ( NGS_FragmentBlob * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcBlob, rcDestroying );
    if ( self != NULL )
    {
        VBlobRelease ( (VBlob*) self -> blob_READ );
        VBlobRelease ( (VBlob*) self -> blob_READ_LEN );
        VBlobRelease ( (VBlob*) self -> blob_READ_TYPE );
        NGS_CursorRelease ( self -> curs, ctx );
        NGS_StringRelease ( self -> run, ctx );
    }
}

static NGS_Refcount_vt NGS_FragmentBlob_vt =
{
    NGS_FragmentBlobWhack
};

NGS_FragmentBlob *
NGS_FragmentBlobMake ( ctx_t ctx, const NGS_String* run, const struct NGS_Cursor* curs, int64_t rowId )
{
    FUNC_ENTRY ( ctx, rcSRA, rcBlob, rcConstructing );
    if ( run == NULL )
    {
        INTERNAL_ERROR ( xcParamNull, "NULL run name" );
    }
    else if ( curs == NULL )
    {
        INTERNAL_ERROR ( xcParamNull, "NULL cursor object" );
    }
    else
    {

        NGS_FragmentBlob * ret = calloc ( 1, sizeof * ret );
        if ( ret == NULL )
        {
            SYSTEM_ERROR ( xcNoMemory, "allocating NGS_FragmentBlob" );
        }
        else
        {
            TRY ( NGS_RefcountInit ( ctx, & ret -> dad, & ITF_Refcount_vt . dad, & NGS_FragmentBlob_vt, "NGS_FragmentBlob", "" ) )
            {
                TRY ( ret -> run = NGS_StringDuplicate ( run, ctx ) )
                {
                    const struct VCursor* vcurs = NGS_CursorGetVCursor ( curs );
                    rc_t rc = VCursorSetRowId ( vcurs, rowId );
                    if ( rc != 0 )
                    {
                        INTERNAL_ERROR ( xcUnexpected, "VCursorSetRowId() rc = %R", rc );
                    }
                    else
                    {
                        rc = VCursorOpenRow ( vcurs );
                        if ( rc != 0 )
                        {
                            INTERNAL_ERROR ( xcUnexpected, "VCursorOpenRow() rc = %R", rc );
                        }
                        else
                        {
                            rc = VCursorGetBlob ( vcurs, & ret -> blob_READ, NGS_CursorGetColumnIndex ( curs, ctx, seq_READ ) );
                            if ( rc != 0  )
                            {
                                VCursorCloseRow ( vcurs );
                                INTERNAL_ERROR ( xcUnexpected, "VCursorGetBlob(READ) rc = %R", rc );
                            }
                            else
                            {
                                rc = VCursorGetBlob ( vcurs, & ret -> blob_READ_LEN, NGS_CursorGetColumnIndex ( curs, ctx, seq_READ_LEN ) );
                                if ( rc != 0  )
                                {
                                    VCursorCloseRow ( vcurs );
                                    INTERNAL_ERROR ( xcUnexpected, "VCursorGetBlob(READ_LEN) rc = %R", rc );
                                }
                                else
                                {
                                    rc = VCursorGetBlob ( vcurs, & ret -> blob_READ_TYPE, NGS_CursorGetColumnIndex ( curs, ctx, seq_READ_TYPE ) );
                                    if ( rc != 0  )
                                    {
                                        VCursorCloseRow ( vcurs );
                                        INTERNAL_ERROR ( xcUnexpected, "VCursorGetBlob(READ_TYPE) rc = %R", rc );
                                    }
                                    else
                                    {
                                        rc = VCursorCloseRow ( vcurs );
                                        if ( rc != 0 )
                                        {
                                            INTERNAL_ERROR ( xcUnexpected, "VCursorCloseRow() rc = %R", rc );
                                        }
                                        else
                                        {
                                            ret -> rowId = rowId;
                                            ret -> curs = NGS_CursorDuplicate ( curs, ctx );
                                            {
                                                /* calculate data/size of the blob (sub-VBlob) starting at rowId */
                                                uint32_t frag_type_elem_bits;
                                                const void *frag_type_base;
                                                uint32_t frag_type_boff;
                                                uint32_t frag_type_row_len;
                                                rc_t rc = VBlobCellData ( ret -> blob_READ,
                                                                        ret -> rowId,
                                                                        & frag_type_elem_bits,
                                                                        & frag_type_base,
                                                                        & frag_type_boff,
                                                                        & frag_type_row_len );
                                                if ( rc != 0 )
                                                {
                                                    INTERNAL_ERROR ( xcUnexpected, "VBlobCellData() rc = %R", rc );
                                                }
                                                else
                                                {
                                                    assert( frag_type_elem_bits == 8 );
                                                    assert( frag_type_boff == 0 );
                                                    ret -> data = frag_type_base;
                                                    ret -> size = BlobBufferBytes ( ret -> blob_READ ) -
                                                                ( (const uint8_t*)( ret -> data ) - (const uint8_t*)( ret -> blob_READ -> data . base ) );
                                                    return ret;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                NGS_FragmentBlobWhack ( ret, ctx );
            }
            free ( ret );
        }
    }

    return NULL;
}

void
NGS_FragmentBlobRelease ( struct NGS_FragmentBlob * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcBlob, rcAccessing );
    if ( self != NULL )
    {
        NGS_RefcountRelease ( & self -> dad, ctx );
    }
}

NGS_FragmentBlob *
NGS_FragmentBlobDuplicate (  struct NGS_FragmentBlob * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcBlob, rcAccessing );
    if ( self != NULL )
    {
        NGS_RefcountDuplicate ( & self -> dad, ctx );
    }
    return self;
}

void
NGS_FragmentBlobRowRange ( const struct NGS_FragmentBlob * self, ctx_t ctx,  int64_t* p_first, uint64_t* p_count )
{
    FUNC_ENTRY ( ctx, rcSRA, rcBlob, rcAccessing );

    if ( self == NULL )
    {
        INTERNAL_ERROR ( xcParamNull, "bad object reference" );
    }
    else
    {
        int64_t first;
        uint64_t count;
        rc_t rc = VBlobIdRange ( self -> blob_READ, & first, & count );
        if ( rc != 0  )
        {
            INTERNAL_ERROR ( xcUnexpected, "VBlobIdRange() rc = %R", rc );
        }
        /* 1st row of VBlob may differ from our first row */
        assert ( first <= self -> rowId );
        if ( p_first != NULL )
        {
            *p_first = self -> rowId;
        }
        if ( p_count != NULL )
        {
            *p_count = count - ( self -> rowId - first );
        }
    }
}

const void*
NGS_FragmentBlobData ( const struct NGS_FragmentBlob * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcBlob, rcAccessing );

    if ( self == NULL )
    {
        INTERNAL_ERROR ( xcParamNull, "bad object reference" );
    }
    else
    {
        return self -> data;
    }
    return 0;
}

uint64_t
NGS_FragmentBlobSize ( const struct NGS_FragmentBlob * self, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcSRA, rcBlob, rcAccessing );

    if ( self == NULL )
    {
        INTERNAL_ERROR ( xcParamNull, "bad object reference" );
    }
    else
    {
        return self -> size;
    }
    return 0;
}

static
void
GetFragInfo ( const NGS_FragmentBlob * self, ctx_t ctx, int64_t p_rowId, uint64_t p_offsetInRow, uint64_t* fragStart, uint64_t* baseCount, int32_t* bioNumber )
{
    FUNC_ENTRY ( ctx, rcSRA, rcDatabase, rcConstructing );
    uint32_t elem_bits;
    const void *base;
    uint32_t boff;
    uint32_t row_len;
    rc_t rc = VBlobCellData ( self -> blob_READ_LEN,
                              p_rowId,
                              & elem_bits,
                              & base,
                              & boff,
                              & row_len );
    if ( rc != 0 )
    {
        INTERNAL_ERROR ( xcUnexpected, "VBlobCellData() rc = %R", rc );
    }
    else
    {
        uint32_t i = 0 ;
        uint64_t offset = 0;
        uint32_t bioFragNum = 0;
        assert ( base != NULL );
        assert ( elem_bits % 8 == 0 );
        assert ( boff == 0 );

        while ( i < row_len )
        {
            uint64_t frag_length;
            switch ( elem_bits )
            {
                case 64:
                {
                    frag_length = ( (const uint64_t*)base ) [ i ];
                    break;
                }
                case 32:
                {
                    frag_length = ( (const uint32_t*)base ) [ i ];
                    break;
                }
                case 16:
                {
                    frag_length = ( (const uint16_t*)base ) [ i ];
                    break;
                }
                case 8:
                {
                    frag_length = ( (const uint8_t*)base ) [ i ];
                    break;
                }
                default:
                {
                    INTERNAL_ERROR ( xcUnexpected, "Unexpected elem_bits: %u", elem_bits );
                    return;
                }
            }

            {
                uint32_t frag_type_elem_bits;
                const void *frag_type_base;
                uint32_t frag_type_boff;
                uint32_t frag_type_row_len;
                rc = VBlobCellData ( self -> blob_READ_TYPE,
                                     p_rowId,
                                     & frag_type_elem_bits,
                                     & frag_type_base,
                                     & frag_type_boff,
                                     & frag_type_row_len );
                if ( rc != 0 )
                {
                    INTERNAL_ERROR ( xcUnexpected, "VBlobCellData() rc = %R", rc );
                }
                else
                {
                    const uint8_t* frag_types = (const uint8_t*)frag_type_base;
                    bool isBiological;
                    assert ( frag_type_row_len == row_len );
                    assert ( frag_type_base != NULL );
                    assert ( frag_type_elem_bits == 8 );
                    assert ( frag_type_boff == 0 );

                    isBiological = frag_types [ i ] & READ_TYPE_BIOLOGICAL;
                    if ( p_offsetInRow < offset + frag_length )
                    {
                        if ( fragStart != NULL )
                        {
                            * fragStart = offset;
                        }
                        if ( baseCount != NULL )
                        {
                            * baseCount = frag_length;
                        }
                        if ( bioNumber != NULL )
                        {
                            * bioNumber = isBiological ? bioFragNum : -1;
                        }
                        return;
                    }

                    if ( isBiological )
                    {
                        ++ bioFragNum;
                    }
                }
            }
            offset += frag_length;
            ++i;
        }
        /* out of fragments */
        INTERNAL_ERROR ( xcUnexpected, "fragment not found in blob: rowId=%li offset=%lu", p_rowId, p_offsetInRow );
    }
}

void
NGS_FragmentBlobInfoByOffset ( const struct NGS_FragmentBlob * self, ctx_t ctx,  uint64_t offsetInBases, int64_t* rowId, uint64_t* fragStart, uint64_t* baseCount, int32_t* bioNumber )
{
    FUNC_ENTRY ( ctx, rcSRA, rcBlob, rcAccessing );
    if ( self == NULL )
    {
        INTERNAL_ERROR ( xcParamNull, "bad object reference" );
    }
    else
    {
        int64_t first;
        uint64_t count;
        rc_t rc = VBlobIdRange ( self -> blob_READ, &first, &count );
        if ( rc != 0  )
        {
            INTERNAL_ERROR ( xcUnexpected, "VBlobIdRange() rc = %R", rc );
        }
        else
        {
            PageMapIterator pmIt;
            rc = PageMapNewIterator ( (const PageMap*)self->blob_READ->pm, &pmIt, 0, count );
            if ( rc != 0 )
            {
                INTERNAL_ERROR ( xcUnexpected, "PageMapNewIterator() rc = %R", rc );
            }
            else
            {
                row_count_t rowInBlob = 0;
                do
                {
                    elem_count_t length = PageMapIteratorDataLength ( &pmIt );
                    elem_count_t offset = PageMapIteratorDataOffset ( &pmIt );
                    row_count_t  repeat = PageMapIteratorRepeatCount ( &pmIt );

                    if ( offsetInBases < offset + length * repeat )
                    {
                        while ( repeat > 1 )
                        {
                            if ( offsetInBases < offset + length )
                            {
                                break;
                            }
                            offset += length;
                            ++rowInBlob;
                        }
                        if ( rowId != NULL )
                        {
                            * rowId = first + rowInBlob + ( offsetInBases - offset ) / length;
                        }
                        GetFragInfo ( self, ctx, *rowId, offsetInBases - offset, fragStart, baseCount, bioNumber );
                        if ( fragStart != NULL )
                        {
                            * fragStart += offset;
                        }
                        break;
                    }
                    ++rowInBlob;
                }
                while ( PageMapIteratorNext ( &pmIt ) );
            }
        }
    }
}

NGS_String*
NGS_FragmentBlobMakeFragmentId ( const struct NGS_FragmentBlob * self, ctx_t ctx, int64_t rowId, uint32_t fragNumber )
{
    FUNC_ENTRY ( ctx, rcSRA, rcBlob, rcAccessing );

    if ( self == NULL )
    {
        INTERNAL_ERROR ( xcParamNull, "bad object reference" );
    }
    else
    {
        return NGS_IdMakeFragment ( ctx, self -> run, false, rowId, fragNumber );
    }
    return NULL;
}
