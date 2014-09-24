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

#ifndef _h_blob_
#define _h_blob_

#ifndef _h_vdb_extern_
 #include <vdb/extern.h>
#endif

#ifndef _h_blob_priv_
 #include "blob-priv.h"
#endif

struct VBlobBuffer;
struct VRowBuffer;

enum VBlobVariant_t {
    vbvBlobbed,
    vbvRowBuffer,
    vbvSingleRow
};


/* Write the row lengths for the rows from start_id thru stop_id, inclusive
 */
rc_t VBlobWritePageMap( VBlob *self, int64_t start_id, int64_t stop_id, const uint32_t row_length[] );

/* Read (copy out) the row lengths for the rows from start_id thru stop_id, inclusive
 */
rc_t VBlobReadPageMap( const VBlob *self, int64_t start_id, int64_t stop_id, uint32_t row_length[] );

/* Change the shape of the databuffer.
 *
 * This function does not resize the buffer; it changes how it is
 * interpreted, i.e. it is a typecast.
 * The quantity ((bits per element) * (# elements)) remains the same.
 *
 * new bits = bits * bits_mult / bits_div
 * new #elements = #elements * bits_div / bits_mult
 * 
 */
rc_t VBlobBufferReshape( struct VBlobBuffer *self, int bits_mult, int bits_div );

/* Turn a raw blob into a page buffer
 *
 * This is essentially a type-cast.
 */
rc_t VBlobMakeRowBuffer( struct VRowBuffer **lhs,
                         const VBlob *cself
);

/* Serialize
 *
 * Get a blob serialized into a new data buffer.
 * Whack this when done with it.
 */
rc_t VBlobSerialize(const VBlob *cself, KDataBuffer *result);


#endif /* _h_blob_ */
