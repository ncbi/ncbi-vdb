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

#ifndef _h_vdb_blob_
#define _h_vdb_blob_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */


/*--------------------------------------------------------------------------
 * VBlob
 *  a collection of data cells
 */
typedef struct VBlob VBlob;


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
VDB_EXTERN rc_t CC VBlobAddRef ( const VBlob *self );
VDB_EXTERN rc_t CC VBlobRelease ( const VBlob *self );


/* IdRange
 *  returns id range for blob
 *
 *  "first" [ OUT, NULL OKAY ] and "count" [ OUT, NULL OKAY ] -
 *  id range is returned in these output parameters, where
 *  at least ONE must be NOT-NULL
 */
VDB_EXTERN rc_t CC VBlobIdRange ( const VBlob *self,
    int64_t *first, uint64_t *count );


/* Read
 *  read entire single cell of byte-aligned data into a buffer
 *
 *  "row_id" [ IN ] - allows ReadDirect random access to any cell
 *  in column
 *
 *  "elem_bits" [ IN ] - expected element size in bits, required
 *  to be compatible with the actual element size, and be a multiple
 *  of 8 ( byte-aligned ). for non-byte-aligned data, see ReadBits
 *
 *  "buffer" [ OUT ] and "blen" [ IN ] - return buffer for row data
 *  where "blen" gives buffer capacity in elements. the total buffer
 *  size in bytes == ( "elem_bits" * "blen" + 7 ) / 8.
 *
 *  "row_len" [ OUT ] - return parameter for the number of elements
 *  in the requested row.
 *
 *  when the return code is 0, "row_len" will contain the number of
 *  elements read into buffer. if the return code indicates that the
 *  buffer is too small, "row_len" will give the required buffer length.
 */
VDB_EXTERN rc_t CC VBlobRead ( const VBlob *self, int64_t row_id,
    uint32_t elem_bits, void *buffer, uint32_t blen, uint32_t *row_len );


/* ReadBits
 *  read single row of potentially bit-aligned cell data into a buffer
 *
 *  "elem_bits" [ IN ] - expected element size in bits, required to be
 *  compatible with the actual element size, and may ( or may not ) be
 *  a multiple of 8 ( byte aligned ).
 *
 *  "start" [ IN ] - zero-based starting index to first element,
 *  valid from 0 .. row_len - 1
 *
 *  "buffer" [ IN ], "boff" [ IN ] and "blen" [ IN ] -
 *  return buffer for row data, where "boff" is in BITS
 *  and "blen" is in ELEMENTS.
 *
 *  "num_read" [ OUT ] - return parameter for the number of elements
 *  read, which is <= "blen"
 *
 *  "remaining" [ OUT, NULL OKAY ] - optional return parameter for
 *  the number of elements remaining to be read. specifically,
 *  "start" + "num_read" + "remaining" == row length, assuming that
 *  "start" <= row length.
 */
VDB_EXTERN rc_t CC VBlobReadBits ( const VBlob *self, int64_t row_id,
    uint32_t elem_bits, uint32_t start, void *buffer, uint32_t boff,
    uint32_t blen, uint32_t *num_read, uint32_t *remaining );


/* CellData
 *  access pointer to single cell of potentially bit-aligned cell data
 *
 *  "elem_bits" [ OUT, NULL OKAY ] - optional return parameter for
 *  element size in bits
 *
 *  "base" [ OUT ] and "boff" [ OUT, NULL OKAY ] -
 *  compound return parameter for pointer to row starting bit
 *  where "boff" is in BITS
 *
 *  "row_len" [ OUT, NULL OKAY ] - the number of elements in cell
 */
VDB_EXTERN rc_t CC VBlobCellData ( const VBlob *self, int64_t row_id,
    uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len );


#ifdef __cplusplus
}
#endif

#endif /*  _h_vdb_blob_ */
