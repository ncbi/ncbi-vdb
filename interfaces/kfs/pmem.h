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

#ifndef _h_kfs_pmem_
#define _h_kfs_pmem_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
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
struct KPageFile;


/*--------------------------------------------------------------------------
 * KMemBank
 *  manages simple blocks of memory
 *  optionally backed by a persistent page file
 */
typedef struct KMemBank KMemBank;


/* Make
 *  make a memory bank with a fixed block size
 *  the total amount of memory may be limited
 *  the memory may be drawn from an externally supplied page file
 *
 *  "block_size" [ IN ] - must be an even multiple of 2, minimum 64
 *
 *  "limit" [ IN, DFLT ZERO ] - the maximum number of bytes
 *  to be allocated zero for unlimited
 *
 *  "backing" [ IN, NULL OKAY ] - a page source for allocator.
 *  a new reference will be added if not null.
 */
KFS_EXTERN rc_t CC KMemBankMake ( KMemBank **bank,
    size_t block_size, uint64_t limit, struct KPageFile *backing );


/* AddRef
 * Release
 */
KFS_EXTERN rc_t CC KMemBankAddRef ( const KMemBank *self );
KFS_EXTERN rc_t CC KMemBankRelease ( const KMemBank *self );


/* Alloc
 *  allocate memory
 *
 *  "id" [ OUT ] - return parameter for memory allocation id
 *
 *  "bytes" [ IN ] - the number of bytes to allocate
 *
 *  "clear" [ IN ] - if true, initialize the memory to zero
 */
KFS_EXTERN rc_t CC KMemBankAlloc ( KMemBank *self,
    uint64_t *id, uint64_t bytes, bool clear );


/* Free
 *  free memory
 *
 *  "id" [ IN, ZERO OKAY ] - id of the allocation to be freed
 */
KFS_EXTERN rc_t CC KMemBankFree ( KMemBank *self, uint64_t id );


/* Size
 *  returns the allocated capacity of the memory object
 *
 *  "id" [ IN ] - id of the allocation
 *
 *  "size" [ OUT ] - return parameter
 */
KFS_EXTERN rc_t CC KMemBankSize ( const KMemBank *self, uint64_t id, uint64_t *size );


/* SetSize
 *  performs a reallocation of memory object
 *
 *  "id" [ IN ] - id of the allocation being resized
 *
 *  "size" [ IN ] - new capacity
 */
KFS_EXTERN rc_t CC KMemBankSetSize ( KMemBank *self, uint64_t id, uint64_t size );


/* Read
 *  linearly access memory
 *
 *  "id" [ IN ] - id of the allocation being read
 *
 *  "pos" [ IN ] - starting offset into allocation
 *
 *  "buffer" [ IN ] and "bsize" [ IN ] - buffer for returned data
 *
 *  "num_read" [ OUT ] - return parameter for the number of bytes read
 */
KFS_EXTERN rc_t CC KMemBankRead ( const KMemBank *self, uint64_t id,
   uint64_t pos, void *buffer, size_t bsize, size_t *num_read );


/* Write
 *  linearly update memory
 *  will resize as required
 *
 *  "id" [ IN ] - id of the allocation being writ
 *
 *  "pos" [ IN ] - starting offset into allocation
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to write
 *
 *  "num_writ" [ OUT ] - return parameter for the number of bytes written
 */
KFS_EXTERN rc_t CC KMemBankWrite ( KMemBank *self, uint64_t id,
    uint64_t pos, const void *buffer, size_t size, size_t *num_writ );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_pmem_ */
