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

#ifndef _h_colfmt_priv_
#define _h_colfmt_priv_

#ifndef _h_kdbfmt_priv_
#include "kdbfmt-priv.h"
#endif

#include <endian.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * KColLocDesc
 *  general location descriptor
 */
typedef struct KColLocDesc KColLocDesc;
struct KColLocDesc
{
    /* the data fork page id */
    uint64_t pg;

    union
    {
        /* pertains to KColBlobLoc */
        struct
        {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            /* the blob size in bytes */
            uint32_t size : 31;

            /* used for level-0 journal */
            uint32_t remove : 1;
#else
            uint32_t remove : 1;
            uint32_t size : 31;
#endif
        } blob;

        /* pertains to KColBlockLoc */
        struct
        {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            /* the block size in bytes */
            uint32_t size : 27;

            /* block type */
            uint32_t id_type : 2;
            uint32_t pg_type : 2;

            /* block compression */
            uint32_t compressed : 1;
#else
            uint32_t compressed : 1;
            uint32_t pg_type : 2;
            uint32_t id_type : 2;
            uint32_t size : 27;
#endif
        } blk;

        /* the general 32 bits */
        uint32_t gen;

    } u;

    /* the id range */
    uint32_t id_range;
    int64_t start_id;
};


/*--------------------------------------------------------------------------
 * KColBlobLoc
 *  a single blob locator
 *  stored in level-0 index
 *  and in various compressed forms in level-2
 */
typedef struct KColLocDesc KColBlobLoc;


/*--------------------------------------------------------------------------
 * KColBlockLoc
 *  an index block locator
 *  stored in level-1 index
 *
 *  the type ids pertain to group representation of { start_id, id_range }
 *  and { pg, size } from KColBlobLoc. An individual KColBlobLoc has no
 *  context and must represent all data. a KColBlockLoc stores grouped locators
 *  and can economize on representations.
 *
 *  there are 4 types of representations applied to each index and data:
 *   0) fully specified "random" access - having { id, range } or { pg, size }
 *      this type can group together locators with no apparent relationship
 *      whatsoever. they are stored within a b-tree ordered by id.
 *
 *   1) uniformly sized discontiguous sequence - any time the range/size is a
 *      constant, then it needs only to be represented once.
 *
 *   2) predictable sequence - when id/pg [ n + 1 ] == id/pg [ n ] +
 *      range/size [ n ] they need not both be stored since the range/size
 *      is a delta of id/pg. the storage format is by magnitude while
 *      the in-core format is by id, with a 32-bit magnitude of the
 *      final entry in a header.
 *
 *   3) uniformly sized contiguous sequence - when the conditions of type #2 apply
 *      and the range/size is always the same, then all values except for the
 *      count are known and may be eliminated.
 *
 *  the specific interpretation of KColBlockLoc depends upon the combination
 *  of id and pg types:
 *
 *   a) when both are "btypePredictable"
 *      "pos"       => idx2 offset to 12 byte block
 *      "size"      => "count", i.e. number of blobs in block
 *      "start_id"  => first id in block
 *      "id_range"  => span of ids, evenly divided into "count" blob indices
 *      "block::pg" => data offset to first blob in block
 *      "block::sz" => size of each blob in block
 *
 *   b) when id type is "btypePredictable":
 *      "pos"       => idx2 offset to dynamically sized block
 *      "size"      => size of block
 *      "start_id"  => first id in block
 *      "id_range"  => span of ids, evenly divided into "count" blob indices
 *      "count"     => functionally derived from pg type and "size"
 *
 *   c) all other cases:
 *      "pos"       => idx2 offset to dynamically sized block
 *      "size"      => size of block
 *      "start_id"  => first id in block
 *      "id_range"  => span of ids
 *      "count"     => functionally derived from id/pg types and "size"
 */
enum
{
    btypeRandom,       /* { pg/id, span/sz } */
    btypeUniform,      /* { pg/id }          */
    btypeMagnitude,    /* { span/sz }        */
    btypePredictable   /* {}                 */
};

typedef struct KColLocDesc KColBlockLoc;


/* KColumnHdr
 *  stored in either "idx1" ( under v1 )
 *  or in "idx" ( v2+ )
 *
 *  in ALL cases, "idx1" starts with at least
 *  a HDBHdr giving the column's current version.
 *  when this version == 1, then the entire header
 *  is in idx1.
 *
 * - ALL VERSIONS -
 *  "dad" - gives endian and version information
 *
 * - VERSION 1 -
 *  "data_eof" - official eof of last segment of data fork
 *
 *  "idx2_eof" - official eof of idx block fork
 *
 *  "num_blocks" - describes the number of idx blocks
 *  in idx2, as well as the number of block descriptors
 *  ( KColBlockLoc ) in idx1.
 *
 *  "page_size" - number of bytes in data pages.
 *  when 1 ( in practice anything less than some higher
 *  power of 2, 64 minimum, 4K typical, 64K etc. ), the
 *  data fork operates in append mode.
 *
 *  "checksum" - one of the blob-wise checksum modes:
 *  kcsNone, kcsCRC32, kcsMD5, tbd...
 *
 *  "align" - round header size out to multiples of 8 bytes
 *
 * - VERSION 3 -
 *  "idx0_count" - a committed entry count on idx0. in v1, this
 *  was always inferred from the file system. in v2, the code
 *  was never writing this value, so it's garbage.
 *
 * - VERSION 4 -
 *  idx1 needs to be written in straight-shot mode when MD5
 *  is involved. in version 3 it had been written in append mode.
 *
 * - VERSION 5 -
 *  "data_limit" - when not zero, imposes a limit for opening
 *  new blobs in append mode and an absolute file size limit
 *  in paged mode ( requiring limit to be a multiple of page_size ).
 *
 *  "data_total" - total size of all segments of data fork
 *
 *  "data_seg" - zero-based id of last data segment
 */
#define KCOL_CURRENT_VERSION 3
#define vCUR v3

typedef struct KColumnHdr KColumnHdr;
struct KColumnHdr
{
    KDBHdr dad;

    union
    {
        /* v1 header */
        struct
        {
            uint64_t data_eof;
            uint64_t idx2_eof;
            uint32_t num_blocks;
            uint32_t page_size;
            uint8_t checksum;

            uint8_t align1 [ 7 ];

        } v1;

        /* v2 header - note one field should be ignored */
        struct
        {
            uint64_t data_eof;
            uint64_t idx2_eof;
            uint32_t ignore;
            uint32_t num_blocks;
            uint32_t page_size;
            uint8_t checksum;

            uint8_t align1 [ 3 ];

        } v2;

        /* v3 header - same as v2, except idx2_count is
           being populated by the implementation */

        /* v4 header - same as v3 in every way
           except that idx1 is expected to be
           written without MD5 append-mode information */
        struct
        {
            uint64_t data_eof;
            uint64_t idx2_eof;
            uint32_t idx0_count;
            uint32_t num_blocks;
            uint32_t page_size;
            uint8_t checksum;

            uint8_t align1 [ 3 ];

        } v3, v4;

        /* v5 header ( proposed ) - supports multi-file data fork */
        struct
        {
            uint64_t data_limit;
            uint64_t data_total;
            uint64_t data_eof;
            uint64_t idx2_eof;
            uint32_t idx0_count;
            uint32_t num_blocks;
            uint32_t page_size;
            uint32_t data_seg;
            uint8_t checksum;

            uint8_t align1 [ 7 ];

        } v5;

    } u;
};

/* NULL_HDR
 *  substitute for a variable when needed
 */
#define NULL_HDR \
    ( * ( const KColumnHdr* ) 0 )

/* KColumnHdrOffset
 *  calculates the idx file offset to beginning of indices
 */
#define KColumnHdrOffset( hdr, var ) \
    ( sizeof ( hdr ) - sizeof ( hdr ) . u + sizeof ( hdr ) . u . var )

/* KColumnHdrMinSize
 *  calculates the minimum size of a header
 */
#define KColumnHdrMinSize( hdr ) \
    sizeof ( hdr ) . dad

/* KColBlockLocEntryCount - INTERNAL
 *  returns the number of entries within a block
 *  uses the original size of the uncompressed block
 *
 *  "orig" [ IN, OUT ] - taken as the original size of an
 *  uncompressed block, but may be updated in the case of
 *  all predictable types.
 */
uint32_t KColBlockLocEntryCount ( const KColBlockLoc *self, size_t *orig );

/* KColBlockLocAllocSize - INTERNAL
 *  returns a size for the block
 *  since a type 3 index ( range/size only ) is nice for storage
 *  but needs to be converted to id/pos for in-memory lookup
 */
size_t KColBlockLocAllocSize ( const KColBlockLoc *self,
    size_t orig, uint32_t count );


#ifdef __cplusplus
}
#endif

#endif /* _h_colfmt_priv_ */
