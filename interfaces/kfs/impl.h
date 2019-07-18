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

#ifndef _h_kfs_impl_
#define _h_kfs_impl_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_kfs_file_impl_
#include <kfs/file-impl.h>
#endif

#ifndef _h_klib_chunk_reader_
#include <kfs/chunk-reader.h>
#endif

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#ifndef _h_kfs_arrayfile_
#include <kfs/arrayfile.h>
#endif

#ifndef _h_klib_namelist_
#include <klib/namelist.h>
#endif

#if KFILE_VERS != 1 && KDIRECTORY_VERS != 1
#error "include <kfs/file-impl.h> and <kfs/directory-impl.h> separately"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KPath;
struct KBufFile;
struct KSysDir_v1;
struct KSysDir_v2;
typedef union KDirectory_vt KDirectory_vt;
typedef union KChunkReader_vt KChunkReader_vt;

#if KDIRECTORY_VERS == 1
#define KSysDir KSysDir_v1
#elif KDIRECTORY_VERS == 2
#define KSysDir KSysDir_v2
#endif


/*--------------------------------------------------------------------------
 * KChunkReader
 *  an interface for driving chunked responses from a single read
 */
struct KChunkReader
{
    const KChunkReader_vt *vt;
    KRefcount refcount;
    uint32_t align;
};

#ifndef KCHUNKREADER_IMPL
#define KCHUNKREADER_IMPL struct KChunkReader
#endif

typedef struct KChunkReader_vt_v1 KChunkReader_vt_v1;
struct KChunkReader_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    rc_t ( CC * destroy ) ( KCHUNKREADER_IMPL *self );
    size_t ( CC * buffer_size ) ( const KCHUNKREADER_IMPL * self );
    rc_t ( CC * next_buffer ) ( KCHUNKREADER_IMPL * self, void ** buf, size_t * size );
    rc_t ( CC * consume_chunk ) ( KCHUNKREADER_IMPL * self, uint64_t pos, const void * buf, size_t size );
    rc_t ( CC * return_buffer ) ( KCHUNKREADER_IMPL * self, void * buf, size_t size );
    /* end minor version == 0 */
};

union KChunkReader_vt
{
    KChunkReader_vt_v1 v1;
};

/* Init
 *  initialize a newly allocated chunk-reader object
 */
KFS_EXTERN rc_t CC KChunkReaderInit ( KChunkReader * self, const KChunkReader_vt *vt );


/*--------------------------------------------------------------------------
 * KDirectory
 *  a virtual directory
 */
struct KDirectory_v1
{
    const KDirectory_vt *vt;
    KRefcount refcount;
    uint8_t read_only;
    uint8_t align [ 3 ];
};

struct KDirectory_v2
{
    const KDirectory_vt *vt;
    KRefcount refcount;
    uint8_t read_only;
    uint8_t align [ 3 ];
};

#ifndef KDIR_IMPL
#define KDIR_IMPL struct KDirectory
#endif

#if KDIRECTORY_VERS == 1
#define KDIRECTORY_V1_IMPL KDIR_IMPL
#define KDIRECTORY_V2_IMPL struct KDirectory_v2
#elif KDIRECTORY_VERS == 2
#define KDIRECTORY_V1_IMPL struct KDirectory_v1
#define KDIRECTORY_V2_IMPL KDIR_IMPL
#else
#error unknown KDirectory version
#endif


typedef struct KDirectory_vt_v1 KDirectory_vt_v1;
struct KDirectory_vt_v1
{
    /* version == 1.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    rc_t ( CC * destroy ) ( KDIR_IMPL *self );
    rc_t ( CC * list_dir ) ( const KDIR_IMPL *self, struct KNamelist **list,
         bool ( CC * f ) ( const KDirectory *dir, const char *name, void *data ),
         void *data, const char *path, va_list args );
    rc_t ( CC * visit ) ( const KDIR_IMPL *self, bool recur,
        rc_t ( CC * f ) ( const KDirectory*, uint32_t, const char*, void* ),
        void *data, const char *path, va_list args );
    rc_t ( CC * visit_update ) ( KDIR_IMPL *self, bool recur,
        rc_t ( CC * f ) ( KDirectory*, uint32_t, const char*, void* ),
        void *data, const char *path, va_list args );
    uint32_t ( CC * path_type ) ( const KDIR_IMPL *self, const char *path, va_list args );
    rc_t ( CC * resolve_path ) ( const KDIR_IMPL *self, bool absolute,
        char *resolved, size_t rsize, const char *path, va_list args );
    rc_t ( CC * resolve_alias ) ( const KDIR_IMPL *self, bool absolute,
        char *resolved, size_t rsize, const char *alias, va_list args );
    rc_t ( CC * rename ) ( KDIR_IMPL *self, bool force, const char *from, const char *to );
    rc_t ( CC * remove ) ( KDIR_IMPL *self, bool force, const char *path, va_list args );
    rc_t ( CC * clear_dir ) ( KDIR_IMPL *self, bool force, const char *path, va_list args );
    rc_t ( CC * access ) ( const KDIR_IMPL *self,
        uint32_t *access, const char *path, va_list args );
    rc_t ( CC * set_access ) ( KDIR_IMPL *self, bool recur,
        uint32_t access, uint32_t mask, const char *path, va_list args );
    rc_t ( CC * create_alias ) ( KDIR_IMPL *self, uint32_t access,
        KCreateMode mode, const char *targ, const char *alias );
    rc_t ( CC * open_file_read ) ( const KDIR_IMPL *self,
        const KFile **f, const char *path, va_list args );
    rc_t ( CC * open_file_write ) ( KDIR_IMPL *self,
        KFile **f, bool update, const char *path, va_list args );
    rc_t ( CC * create_file ) ( KDIR_IMPL *self, KFile **f, bool update,
        uint32_t access, KCreateMode mode, const char *path, va_list args );
    rc_t ( CC * file_size ) ( const KDIR_IMPL *self,
        uint64_t *size, const char *path, va_list args );
    rc_t ( CC * set_size ) ( KDIR_IMPL *self,
        uint64_t size, const char *path, va_list args );
    rc_t ( CC * open_dir_read ) ( const KDIR_IMPL *self,
        const KDirectory **sub, bool chroot, const char *path, va_list args );
    rc_t ( CC * open_dir_update ) ( KDIR_IMPL *self,
        KDirectory **sub, bool chroot, const char *path, va_list args );
    rc_t ( CC * create_dir ) ( KDIR_IMPL *self, uint32_t access,
        KCreateMode mode, const char *path, va_list args );

    /* optional destructor method - leave NULL if not needed */
    rc_t ( CC * destroy_file ) ( KDIR_IMPL *self, KFile *f );
    /* end minor version == 0 */

    /* start minor version == 1 */
    rc_t ( CC * date ) ( const KDIR_IMPL *self,
        KTime_t * date, const char *path, va_list args );
    rc_t ( CC * setdate ) ( KDIR_IMPL * self, bool recur,
        KTime_t date, const char *path, va_list args );
    struct KSysDir_v1* ( CC * get_sysdir ) ( const KDIR_IMPL *self );
    /* end minor version == 1 */

    /* start minor version == 2 */
    rc_t ( CC * file_locator ) ( const KDIR_IMPL *self,
        uint64_t *locator, const char *path, va_list args );
    /* end minor version == 2 */

    /* start minor version == 3 */
    rc_t ( CC * file_phys_size ) ( const KDIR_IMPL *self,
        uint64_t *phys_size, const char *path, va_list args );
    rc_t ( CC * file_contiguous ) ( const KDIR_IMPL *self,
        bool *contiguous, const char *path, va_list args );
    /* end minor version == 3 */

    /* start minor version == 4 */
    rc_t ( CC * open_file_shared_write ) ( KDIR_IMPL *self,
        KFile **f, bool update, const char *path, va_list args );
    /* end minor version == 4 */

    /* ANY NEW ENTRIES MUST BE REFLECTED IN libs/kfs/directory.c
       BY BOTH THE CORRESPONDING MESSAGE DISPATCH FUNCTION(s) AND
       VTABLE VALIDITY CHECKS IN KDirectoryInit */
};

typedef struct KDirectory_vt_v2 KDirectory_vt_v2;
struct KDirectory_vt_v2
{
    /* version == 2.x */
    uint32_t maj;
    uint32_t min;

    /* start minor version == 0 */
    void ( CC * destroy ) ( KDIRECTORY_V2_IMPL *self, ctx_t ctx );
    void ( CC * visit ) ( const KDIRECTORY_V2_IMPL *self, ctx_t ctx, bool recur,
        bool ( CC * f ) ( const KDirectory_v2 *dir, uint32_t type, const char *name, void *data ),
        void *data, struct KPath const *path );
    struct KPath * ( CC * resolve_path ) ( const KDIRECTORY_V2_IMPL *self, ctx_t ctx, bool absolute,
        struct KPath const *path );
    void ( CC * rename ) ( KDIRECTORY_V2_IMPL *self, ctx_t ctx, bool force, struct KPath const *from, struct KPath const *to );
    void ( CC * remove ) ( KDIRECTORY_V2_IMPL *self, ctx_t ctx, bool force, struct KPath const *path );
    void ( CC * clear_dir ) ( KDIRECTORY_V2_IMPL *self, ctx_t ctx, bool force, struct KPath const *path );
    void ( CC * create_alias ) ( KDIRECTORY_V2_IMPL *self, ctx_t ctx, uint32_t access,
        KCreateMode mode, struct KPath const *targ, struct KPath const *alias );
    struct KFile_v2 * ( CC * create_file ) ( KDIRECTORY_V2_IMPL *self, ctx_t ctx, bool update,
        uint32_t access, KCreateMode mode, struct KPath const *path );
    void ( CC * create_dir ) ( KDIRECTORY_V2_IMPL *self, ctx_t ctx, uint32_t access,
        KCreateMode mode, struct KPath const *path );

    /* ANY NEW ENTRIES MUST BE REFLECTED IN libs/kfs/directory.c
       BY BOTH THE CORRESPONDING MESSAGE DISPATCH FUNCTION(s) AND
       VTABLE VALIDITY CHECKS IN KDirectoryInit */
};

union KDirectory_vt
{
    KDirectory_vt_v1 v1;
    KDirectory_vt_v2 v2;
};

/* Init
 *  initialize a newly allocated directory object
 */
KFS_EXTERN rc_t CC KDirectoryInit_v1 ( KDirectory_v1 *self, const KDirectory_vt *vt, 
    const char * class_name, const char * path, bool update );

/* DestroyFile
 *  does whatever is necessary with an unreferenced file
 */
KFS_EXTERN rc_t CC KDirectoryDestroyFile_v1 ( const KDirectory_v1 *self, KFile_v1 *f );
KFS_EXTERN void CC KDirectoryDestroyFile_v2 ( const KDirectory_v2 *self, ctx_t ctx, KFile_v2 *f );

/* GetSysDir
 *  returns an underlying system file object
 */
KFS_EXTERN struct KSysDir_v1* CC KDirectoryGetSysDir_v1 ( const KDirectory_v1 *self );
KFS_EXTERN struct KSysDir_v2* CC KDirectoryGetSysDir_v2 ( const KDirectory_v2 *self, ctx_t ctx );


/* RealPath
 *  exposes functionality of system directory
 */
KFS_EXTERN rc_t CC KSysDirRealPath_v1 ( struct KSysDir_v1 const *self,
    char *real, size_t bsize, const char *path, ... );
KFS_EXTERN rc_t CC KSysDirVRealPath ( struct KSysDir_v1 const *self,
    char *real, size_t bsize, const char *path, va_list args );

#define KDirectoryInit NAME_VERS ( KDirectoryInit, KFILE_VERS )
#define KDirectoryDestoryFile NAME_VERS ( KDirectoryDestoryFile, KFILE_VERS )
#define KDirectoryGetSysDir NAME_VERS ( KDirectoryGetSysDir, KFILE_VERS )
#define KSysDirRealPath NAME_VERS ( KSysDirRealPath, KFILE_VERS )


/*--------------------------------------------------------------------------
 * KArrayFile
 *  an array-file is created from a KFile
 */
typedef union  KArrayFile_vt KArrayFile_vt;
struct KArrayFile
{
    const KArrayFile_vt *vt;
    KRefcount refcount;
    uint8_t read_enabled;
    uint8_t write_enabled;
    uint8_t align [ 2 ];
};

#ifndef KARRAYFILE_IMPL
#define KARRAYFILE_IMPL KArrayFile
#endif

typedef struct KArrayFile_vt_v1 KArrayFile_vt_v1;
struct KArrayFile_vt_v1
{
    /* version number */
    uint32_t maj, min;

    /* start minor version == 0 */
    rc_t ( CC * destroy ) ( KARRAYFILE_IMPL *self );
    rc_t ( CC * dimensionality ) ( const KARRAYFILE_IMPL *self, uint8_t *dim );
    rc_t ( CC * set_dimensionality ) ( KARRAYFILE_IMPL *self, uint8_t dim );
    rc_t ( CC * dim_extents ) ( const KARRAYFILE_IMPL *self, uint8_t dim, uint64_t *extents );
    rc_t ( CC * set_dim_extents ) ( KARRAYFILE_IMPL *self, uint8_t dim, uint64_t *extents );
    rc_t ( CC * element_size ) ( const KARRAYFILE_IMPL *self, uint64_t *elem_bits );
    rc_t ( CC * read ) ( const KARRAYFILE_IMPL *self, uint8_t dim,
        const uint64_t *pos, void *buffer, const uint64_t *elem_count,
        uint64_t *num_read );
    rc_t ( CC * write ) ( KARRAYFILE_IMPL *self, uint8_t dim,
        const uint64_t *pos, const void *buffer, const uint64_t *elem_count,
        uint64_t *num_writ );
    rc_t ( CC * get_meta ) ( const KARRAYFILE_IMPL *self, const char *key, 
                             const KNamelist **list );

    rc_t ( CC * read_v ) ( const KARRAYFILE_IMPL *self, uint8_t dim,
        const uint64_t * pos, char * buffer, const uint64_t buffer_size,
        uint64_t * num_read );

    /* end minor version == 0 */
};

union KArrayFile_vt
{
    KArrayFile_vt_v1 v1;
};

/* Init
 *  initialize a newly allocated array-file object
 */
KFS_EXTERN rc_t CC KArrayFileInit ( KArrayFile *self, const KArrayFile_vt *vt, 
    bool read_enabled, bool write_enabled );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_impl_ */
