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

#ifndef _h_kfs_pagefile_
#define _h_kfs_pagefile_

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
 * KPage
 *  a reference counted page
 */
typedef struct KPage KPage;


/* AddRef
 * Release
 *  ignores NULL references
 */
KFS_EXTERN rc_t CC KPageAddRef ( const KPage *self );
KFS_EXTERN rc_t CC KPageRelease ( const KPage *self );


/* Id
 *  returns page id
 *
 *  "page_id" [ OUT ] - return parameter for page id
 */
KFS_EXTERN rc_t CC KPageId ( const KPage *self, uint32_t *page_id );


/* ConstSize
 *  returns constant page size
 */
KFS_EXTERN size_t CC KPageConstSize ( void );


/* AccessRead
 * AccessUpdate
 *  gain access to page memory
 *  update access marks page as modified
 *
 *  "mem" [ OUT ] - pointer to page
 *
 *  "bytes" [ OUT, NULL OKAY ] - page size in bytes
 */
KFS_EXTERN rc_t CC KPageAccessRead ( const KPage *self, const void **mem, size_t *bytes );
KFS_EXTERN rc_t CC KPageAccessUpdate ( KPage *self, void **mem, size_t *bytes );


/*--------------------------------------------------------------------------
 * KPageFile
 *  presents some level of page management on top of a random-access KFile
 */
typedef struct KPageFile KPageFile;


/* Make
 *  creates a page file
 *
 *  "pf" [ OUT ] - return parameter for page file
 *
 *  "backing" [ IN ] - backing file
 *   NB - attaches a new reference to file
 *        does not take ownership
 *
 *  "climit" [ IN ] - cache size limit
 *
 *  "write_through" [ IN ] - if true, causes page flushing
 *  on each KPageRelease message if modified.
 */
KFS_EXTERN rc_t CC KPageFileMakeRead ( const KPageFile **pf,
    struct KFile const *backing, size_t climit );
KFS_EXTERN rc_t CC KPageFileMakeUpdate ( KPageFile **pf,
    struct KFile *backing, size_t climit, bool write_through );


/* AddRef
 * Release
 *  ignores NULL references
 */
KFS_EXTERN rc_t CC KPageFileAddRef ( const KPageFile *self );
KFS_EXTERN rc_t CC KPageFileRelease ( const KPageFile *self );


/* Size
 *  returns size in bytes of file and cache
 *
 *  "lsize" [ OUT, NULL OKAY ] - return parameter for logical size
 *
 *  "fsize" [ OUT, NULL OKAY ] - return parameter for file size
 *
 *  "csize" [ OUT, NULL OKAY ] - return parameter for cache size
 */
KFS_EXTERN rc_t CC KPageFileSize ( const KPageFile *self,
    uint64_t *lsize, uint64_t *fsize, size_t *csize );


/* SetSize
 *  extends or truncates underlying file
 *  may affect cache contents
 *
 *  "size" [ IN ] - logical size
 */
KFS_EXTERN rc_t CC KPageFileSetSize ( KPageFile *self, uint64_t size );


/* Alloc
 *  allocates a new page
 *  the page will be zeroed and initially unmodified
 *
 *  "page" [ OUT ] - return parameter for page object
 *
 *  "page_id" [ OUT, NULL OKAY ] - optional return parameter for page id
 */
KFS_EXTERN rc_t CC KPageFileAlloc ( KPageFile *self, KPage **page, uint32_t *page_id );


/* Get
 *  returns an existing page
 *
 *  "page" [ OUT ] - return parameter for page object
 *
 *  "page_id" [ IN ] - id of page to retrieve
 */
KFS_EXTERN rc_t CC KPageFileGet ( KPageFile *self, KPage **page, uint32_t page_id );


/* PosGet
 *  returns a page corresponding to position
 *
 *  "page" [ OUT ] - return parameter for page object
 *
 *  "offset" [ IN ] - offset to a byte within file
 */
KFS_EXTERN rc_t CC KPageFilePosGet ( KPageFile *self, KPage **page, uint64_t offset );


/* DropBacking
 *  used immediately prior to releasing
 *  prevents modified pages from being flushed to disk
 *  renders object nearly useless
 */
KFS_EXTERN rc_t CC KPageFileDropBacking ( KPageFile *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_pagefile_ */
