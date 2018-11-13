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

#ifndef _h_kfs_file_impl_
#define _h_kfs_file_impl_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_kfs_file_
#include <kfs/file.h>
#endif

#ifndef _h_kfc_refcount_impl_
#include <kfc/refcount-impl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct timeout_t;
struct KSysFile_v1;
struct KSysFile_v2;
struct KDirectory_v1;
typedef union KFile_vt KFile_vt;
extern KItfTok KFile_tok_v2;

#if KFILE_VERS == 1
#define KSysFile KSysFile_v1
#elif KFILE_VERS == 2
#define KSysFile KSysFile_v2
#endif


/*--------------------------------------------------------------------------
 * KFile
 *  a virtual file
 */
struct KFile_v1
{
    const KFile_vt *vt;
    struct KDirectory_v1 const *dir;
    KRefcount refcount;
    uint8_t read_enabled;
    uint8_t write_enabled;
    uint8_t align [ 2 ];
};

struct KFile_v2
{
    KRefcount_v1 dad;
    uint8_t read_enabled;
    uint8_t write_enabled;
    uint8_t align [ sizeof ( void* ) - 2 ];
};

#ifndef KFILE_IMPL
#define KFILE_IMPL struct KFile_v1
#endif

typedef struct KFile_vt_v1 KFile_vt_v1;
struct KFile_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    rc_t ( CC * destroy ) ( KFILE_IMPL *self );
    struct KSysFile_v1* ( CC * get_sysfile ) ( const KFILE_IMPL *self, uint64_t *offset );
    rc_t ( CC * random_access ) ( const KFILE_IMPL *self );
    rc_t ( CC * get_size ) ( const KFILE_IMPL *self, uint64_t *size );
    rc_t ( CC * set_size ) ( KFILE_IMPL *self, uint64_t size );
    /* num_read and num_writ are guaranteed non-NULL */
    rc_t ( CC * read ) ( const KFILE_IMPL *self, uint64_t pos,
        void *buffer, size_t bsize, size_t *num_read );
    rc_t ( CC * write ) ( KFILE_IMPL *self, uint64_t pos,
        const void *buffer, size_t size, size_t *num_writ );
    /* end minor version == 0 */

    /* start minor version == 1 */
    uint32_t ( CC * get_type ) ( const KFILE_IMPL * self );
    /* end minor version == 1 */

    /* start minor version == 2 */
    rc_t ( CC * timed_read ) ( const KFILE_IMPL *self, uint64_t pos,
        void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm );
    rc_t ( CC * timed_write ) ( KFILE_IMPL *self, uint64_t pos,
        const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm );
    /* end minor version == 2 */

    /* start minor version == 3 */
    rc_t ( CC * read_chunked ) ( const KFILE_IMPL *self, uint64_t pos,
        struct KChunkReader * chunks, size_t bsize, size_t * num_read );
    rc_t ( CC * timed_read_chunked ) ( const KFILE_IMPL *self, uint64_t pos,
        struct KChunkReader * chunks, size_t bsize, size_t * num_read, struct timeout_t * tm );
    /* end minor version == 3 */

    /* ANY NEW ENTRIES MUST BE REFLECTED IN libs/kfs/file.c
       BY BOTH THE CORRESPONDING MESSAGE DISPATCH FUNCTION(s) AND
       VTABLE VALIDITY CHECKS IN KFileInit_v1 */
};

union KFile_vt
{
    KFile_vt_v1 v1;
};

typedef struct KFile_v2_vt KFile_v2_vt;
struct KFile_v2_vt
{
    KVTable dad;

    /* start minor version == 0 */
    struct KSysFile_v2* ( CC * get_sysfile ) ( const KFile_v2 *self, ctx_t ctx, uint64_t *offset );
    bool ( CC * random_access ) ( const KFile_v2 *self, ctx_t ctx );
    uint64_t ( CC * get_size ) ( const KFile_v2 *self, ctx_t ctx );
    void ( CC * set_size ) ( KFile_v2 *self, ctx_t ctx, uint64_t size );
    /* num_read and num_writ are guaranteed non-NULL */
    size_t ( CC * read ) ( const KFile_v2 *self, ctx_t ctx, uint64_t pos,
        void *buffer, size_t bsize );
    size_t ( CC * write ) ( KFile_v2 *self, ctx_t ctx, uint64_t pos,
        const void *buffer, size_t size );

    uint32_t ( CC * get_type ) ( const KFile_v2 * self, ctx_t ctx );

    size_t ( CC * timed_read ) ( const KFile_v2 *self, ctx_t ctx, uint64_t pos,
        void *buffer, size_t bsize, struct timeout_t *tm );
    size_t ( CC * timed_write ) ( KFile_v2 *self, ctx_t ctx, uint64_t pos,
        const void *buffer, size_t size, struct timeout_t *tm );

    /* ANY NEW ENTRIES MUST BE REFLECTED IN libs/kfs/file-v2.c
       BY BOTH THE CORRESPONDING MESSAGE DISPATCH FUNCTION(s) AND
       VTABLE VALIDITY CHECKS IN KFileInit_v2 */
};


/* Init
 *  initialize a newly allocated file object
 */
KFS_EXTERN rc_t CC KFileInit_v1 ( KFile_v1 *self, const KFile_vt *vt,
    const char *classname, const char *fname,
    bool read_enabled, bool write_enabled );
KFS_EXTERN void CC KFileInit_v2 ( KFile_v2 *self, ctx_t ctx, const KVTable *vt,
    const char *fname, bool read_enabled, bool write_enabled );

/* Destroy
 *  destroy file
 */
KFS_EXTERN rc_t CC KFileDestroy_v1 ( KFile_v1 *self );
KFS_EXTERN void CC KFileDestroy_v2 ( KFile_v2 * self );

/* GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 */
KFS_EXTERN struct KSysFile_v1 * CC KFileGetSysFile_v1 ( const KFile_v1 *self, uint64_t *offset );
KFS_EXTERN struct KSysFile_v2 * CC KFileGetSysFile_v2 ( const KFile_v2 *self, ctx_t ctx, uint64_t *offset );

#define KFileInit NAME_VERS ( KFileInit, KFILE_VERS )
#define KFileDestroy NAME_VERS ( KFileDestory, KFILE_VERS )
#define KFileGetSysFile NAME_VERS ( KFileGetSysFile, KFILE_VERS )


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_file_impl_ */
