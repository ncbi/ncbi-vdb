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

#ifndef _h_kfs_sparsefile_
#define _h_kfs_sparsefile_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_kfc_refcount_
#include <kfc/refcount.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile_v1;
struct KFile_v2;


/*--------------------------------------------------------------------------
 * KSparseFile
 *  a file that will exhibit sparse behavior
 */
typedef struct KSparseFile_v1 KSparseFile_v1;
typedef struct KSparseFile_v2 KSparseFile_v2;

/* Make
 *  make a sparse file from an existing KFile
 *  the existing file must be either empty or already a sparse file
 *
 *  "sparse" [ OUT ] - return parameter for sparse file
 *
 *  "dense" [ IN ] - backing file for dense data. must either be empty
 *   or a variable-size sparse file with the same block size
 *
 *  "block_size" [ IN ] - size of file blocks, e.g. 128 * 1024.
 *   must be an even power of 2.
 */
KFS_EXTERN rc_t CC KSparseFileMake_v1 ( KSparseFile_v1 ** sparse, struct KFile_v1 * dense, size_t block_size );
KFS_EXTERN KSparseFile_v2 * CC KSparseFileMake_v2 ( ctx_t ctx, struct KFile_v2 * dense, size_t block_size );


/* MakeFixed
 *  make a fixed-size sparse file from an existing KFile
 *  the existing file must be either empty or already a fixed-size sparse file of same size
 *
 *  "sparse" [ OUT ] - return parameter for sparse file
 *
 *  "dense" [ IN ] - backing file for dense data. must either be empty
 *   or a fixed-size sparse file with the same logical and block sizes
 *
 *  "block_size" [ IN ] - size of file blocks, e.g. 128 * 1024.
 *   must be an even power of 2.
 *
 *  "logical_size" [ IN ] - end of file in bytes.
 *   not required to be power of 2.
 */
KFS_EXTERN rc_t CC KSparseFileMakeFixed_v1 ( KSparseFile_v1 ** sparse, struct KFile_v1 * dense,
    size_t block_size, uint64_t logical_size );
KFS_EXTERN KSparseFile_v2 * CC KSparseFileMakeFixed_v2 ( ctx_t ctx, struct KFile_v2 * dense,
    size_t block_size, uint64_t logical_size );


/* AddRef
 * Release
 *  for KFile_v1
 *  resistant to NULL self references
 */
KFS_EXTERN rc_t CC KSparseFileAddRef_v1 ( const KSparseFile_v1 * self );
KFS_EXTERN rc_t CC KSparseFileRelease_v1 ( const KSparseFile_v1 * self );

/* Duplicate
 * Release
 *  for KFile_v2
 *  ignores NULL references
 */
static __inline__
KSparseFile_v2 * KSparseFileDuplicate_v2 ( const KSparseFile_v2 * self, ctx_t ctx, caps_t rm )
{
    return ( KSparseFile_v2 * ) KRefcountDuplicate_v1 ( TO_REFCOUNT_V1 ( self ), ctx, rm );
}

static __inline__
void KSparseFileRelease_v2 ( const KSparseFile_v2 * self, ctx_t ctx )
{
    KRefcountRelease_v1 ( TO_REFCOUNT_V1 ( self ), ctx );
}


/* ToKFile
 *  cast the sparse file to a normal KFile
 *  returns a new reference to the KFile
 */
KFS_EXTERN rc_t CC KSparseFileToKFile_v1 ( const KSparseFile_v1 * self, struct KFile_v1 ** file );
KFS_EXTERN struct KFile_v2 * CC KSparseFileToKFile_v2 ( const KSparseFile_v2 * self, ctx_t ctx );



/*--------------------------------------------------------------------------
 * versioning
 */
#define KSparseFile NAME_VERS ( KSparseFile, KFILE_VERS )
#define KSparseFileMake NAME_VERS ( KSparseFileMake, KFILE_VERS )
#define KSparseFileMakeFixed NAME_VERS ( KSparseFileMakeFixed, KFILE_VERS )
#define KSparseFileAddRef NAME_VERS ( KSparseFileAddRef, KFILE_VERS )
#define KSparseFileDuplicate NAME_VERS ( KSparseFileDuplicate, KFILE_VERS )
#define KSparseFileRelease NAME_VERS ( KSparseFileRelease, KFILE_VERS )
#define KSparseFileToKFile NAME_VERS ( KSparseFileToKFile, KFILE_VERS )


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_sparsefile_ */
