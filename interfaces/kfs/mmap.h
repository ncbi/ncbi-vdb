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

#ifndef _h_kfs_mmap_
#define _h_kfs_mmap_

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
struct KFile;


/*--------------------------------------------------------------------------
 * KMMap
 *  a memory mapped region
 */
typedef struct KMMap KMMap;

/* AddRef
 * Release
 *  ignores NULL references
 */
KFS_EXTERN rc_t CC KMMapAddRef ( const KMMap *self );
KFS_EXTERN rc_t CC KMMapRelease ( const KMMap *self );

/* Size
 *  returns size of memory region
 *
 *  "size" [ OUT ] - return parameter for region size
 */
KFS_EXTERN rc_t CC KMMapSize ( const KMMap *self, size_t *size );

/* Position
 *  returns region starting offset into mapped file
 *
 *  "pos" [ OUT ] - return parameter for file offset
 */
KFS_EXTERN rc_t CC KMMapPosition ( const KMMap *self, uint64_t *pos );

/* Reposition
 *  shift region to a new position
 *  may change region size
 *
 *  "pos" [ IN ] - new starting position for region
 *
 *  "size" [ OUT, NULL OKAY ] - optional return parameter
 *  for resultant region size
 */
KFS_EXTERN rc_t CC KMMapReposition ( const KMMap *self, uint64_t pos, size_t *size );

/* Addr
 *  returns starting address of memory region
 *
 *  "addr" [ OUT ] - return parameter for memory address
 */
KFS_EXTERN rc_t CC KMMapAddrRead ( const KMMap *self, const void **addr );
KFS_EXTERN rc_t CC KMMapAddrUpdate ( KMMap *self, void **addr );

/* Make
 *  maps entire file
 *
 *  "f" [ IN ] - file to map.
 *  a new reference will be added
 */
KFS_EXTERN rc_t CC KMMapMakeRead ( const KMMap **mm, struct KFile const *f );
KFS_EXTERN rc_t CC KMMapMakeUpdate ( KMMap **mm, struct KFile *f );

/* MakeMax
 *  maps as much of a file as possible
 *
 *  "f" [ IN ] - file to map.
 *  a new reference will be added
 */
KFS_EXTERN rc_t CC KMMapMakeMaxRead ( const KMMap **mm, struct KFile const *f );
KFS_EXTERN rc_t CC KMMapMakeMaxUpdate ( KMMap **mm, struct KFile *f );

/* MakeRgn
 *  maps a portion of a file
 *
 *  "f" [ IN ] - file to map
 *  a new reference will be added
 *
 *  "pos" [ IN ] - starting offset into file
 *
 *  "size" [ IN, DEFAULT ZERO ] - size of resulting region. if 0,
 *  the size will be interpreted as meaning size of "f" - "pos".
 */
KFS_EXTERN rc_t CC KMMapMakeRgnRead ( const KMMap **mm,
    struct KFile const *f, uint64_t pos, size_t size );
KFS_EXTERN rc_t CC KMMapMakeRgnUpdate ( KMMap **mm,
    struct KFile *f, uint64_t pos, size_t size );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_mmap_ */
