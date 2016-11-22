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

#include "VByteBlob.h"

#include <kfc/except.h>
#include <kfc/xc.h>

#include <vdb/blob.h>
#include <../libs/vdb/blob-priv.h>
#include <../libs/vdb/page-map.h>

/* Calculate the biggest available contiguous data portion of the blob:
*  starts at rowId, ends before a repeated value or at the end of the blob
*/
void
VByteBlob_ContiguousChunk ( const VBlob* p_blob,  ctx_t ctx, int64_t rowId, uint64_t p_maxRows, const void** p_data, uint64_t* p_size, bool p_stopAtRepeat )
{
    FUNC_ENTRY ( ctx, rcSRA, rcBlob, rcAccessing );

    assert ( p_blob );
    assert ( p_data );
    assert ( p_size );

    {
        uint32_t elem_bits;
        const void *base;
        uint32_t boff;
        uint32_t row_len;
        rc_t rc = VBlobCellData ( p_blob,
                                  rowId,
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
			rc_t rc;
            int64_t first;
            uint64_t count;

            assert( elem_bits == 8 );
            assert( boff == 0 );
            *p_data = base;
            *p_size = 0;

            rc = VBlobIdRange ( p_blob, & first, & count );
            if ( rc != 0  )
            {
                INTERNAL_ERROR ( xcUnexpected, "VBlobIdRange() rc = %R", rc );
            }
            else if ( p_stopAtRepeat )
            {
                PageMapIterator pmIt;

                assert ( rowId >= first && rowId < first + (int64_t)count );

                if ( rowId - first + 1 < (int64_t)count ) /* more rows in the blob */
                {   /* *p_size is the size of value on rowId. Increase size to include subsequent rows, until we see a repeat, p_maxRows is reached or the blob ends */
                    rc = PageMapNewIterator ( (const PageMap*)p_blob->pm, &pmIt, rowId - first, count - ( rowId - first ) ); /* here, rowId is relative to the blob */
                    if ( rc != 0 )
                    {
                        INTERNAL_ERROR ( xcUnexpected, "PageMapNewIterator() rc = %R", rc );
                    }
                    else
                    {
                        do
                        {
                            *p_size += PageMapIteratorDataLength ( &pmIt );
                            row_count_t  repeat = PageMapIteratorRepeatCount ( &pmIt );
                            if ( p_maxRows != 0 )
                            {
                                if ( repeat < p_maxRows )
                                {
                                    p_maxRows -= repeat;
                                }
                                else
                                {   /* p_maxRows reached */
                                    break;
                                }
                            }
                            if ( PageMapIteratorRepeatCount ( &pmIt ) > 1 )
                            {   /* repeated row found */
                                break;
                            }

                        }
                        while ( PageMapIteratorNext ( &pmIt ) );
                    }
                }
                else
                {
                    *p_size = row_len;
                }
            }
            else if ( p_maxRows > 0 && p_maxRows < count - ( rowId - first ) )
            {   /* return the size of the first p_maxRows rows */
                const uint8_t* firstRow = (const uint8_t*)base;
                rc = VBlobCellData ( p_blob,
                                     rowId + p_maxRows,
                                     & elem_bits,
                                     & base,
                                     & boff,
                                     & row_len );
                *p_size = (const uint8_t*)( base ) - firstRow;
            }
            else
            {   /* set the size to include the rest of the blob's data */
                *p_size = BlobBufferBytes ( p_blob ) - ( (const uint8_t*)( base ) - (const uint8_t*)( p_blob -> data . base ) );
            }
        }
    }
}

