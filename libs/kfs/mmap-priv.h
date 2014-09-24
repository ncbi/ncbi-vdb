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

#ifndef _h_mmap_priv_
#define _h_mmap_priv_

#ifndef _h_kfs_mmap_
#include <kfs/mmap.h>
#endif

#ifndef _h_kfs_file_
#include <kfs/file.h>
#endif

#ifndef _h_klib_refcount_
#include <klib/refcount.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * KMMap
 *  a memory mapped region
 */
#if 0
/* minimal structure */
struct KMMap
{
    uint64_t off;
    uint64_t pos;
    size_t size;
    char *addr;

    KFile *f;
    size_t pg_size;

    uint32_t addr_adj;
    uint32_t size_adj;

    KRefcount refcount;

    bool read_only;
    bool sys_mmap;
    bool dirty;
};
#endif


/* Make
 *  make an open OS-specific KMMap structure
 *  common code will initialize common parts such as KFile*
 */
rc_t KMMapMake ( KMMap **mmp );


/* RWSys
 *  map the region of mapped file into memory for read & write
 *  using system memory mapping function
 *
 *  the values for "pos" and "size" can be trusted
 *  as correct for page alignment
 */
rc_t KMMapRWSys ( KMMap *self, uint64_t pos, size_t size );


/* ROSys
 *  map the region of mapped file into memory for read only
 *  using system memory mapping function
 *
 *  the values for "pos" and "size" can be trusted
 *  as correct for page alignment
 */
rc_t KMMapROSys ( KMMap *self, uint64_t pos, size_t size );


/* Unmap
 *  removes a memory map
 */
rc_t KMMapUnmap ( KMMap *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_mmap_priv_ */
