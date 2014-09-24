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

#ifndef _h_kfs_arrayfile_
#define _h_kfs_arrayfile_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_klib_namelist_
#include <klib/namelist.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct KFile;


/*--------------------------------------------------------------------------
 * KArrayFile
 *  an array-file is created from a KFile
 */
typedef struct KArrayFile KArrayFile;


/* Make
 *  make an array file from a KFile
 */
/*
KFS_EXTERN rc_t CC KArrayFileMakeRead ( const KArrayFile **af, struct KFile const *file );
KFS_EXTERN rc_t CC KArrayFileMakeUpdate ( KArrayFile **af, struct KFile *file );
*/


/* AddRef
 * Release
 *  ignores NULL references
 */
KFS_EXTERN rc_t CC KArrayFileAddRef ( const KArrayFile *self );
KFS_EXTERN rc_t CC KArrayFileRelease ( const KArrayFile *self );


/* Dimensionality
 *  returns the number of dimensions in the ArrayFile
 *
 *  "dim" [ OUT ] - return parameter for number of dimensions
 */
KFS_EXTERN rc_t CC KArrayFileDimensionality ( const KArrayFile *self, uint8_t *dim );


/* SetDimensionality
 *  sets the number of dimensions in the ArrayFile
 *
 *  "dim" [ IN ] - new number of dimensions; must be > 0
 */
KFS_EXTERN rc_t CC KArrayFileSetDimensionality ( KArrayFile *self, uint8_t dim );


/* DimExtents
 *  returns the extent of every dimension
 *
 *  "dim" [ IN ] - the dimensionality of "extents"
 *
 *  "extents" [ OUT ] - returns the extent for every dimension
 */
KFS_EXTERN rc_t CC KArrayFileDimExtents ( const KArrayFile *self, uint8_t dim, uint64_t *extents );


/* SetDimExtents
 *  sets the new extents for every dimension
 *
 *  "dim" [ IN ] - the dimensionality of "extents"
 *
 *  "extents" [ IN ] - new extents for every dimension
 */
KFS_EXTERN rc_t CC KArrayFileSetDimExtents ( KArrayFile *self, uint8_t dim, uint64_t *extents );


/* ElementSize
 *  returns the element size in bits
 *
 *  "elem_bits" [ OUT ] - size of each element in bits
 */
KFS_EXTERN rc_t CC KArrayFileElementSize ( const KArrayFile *self, uint64_t *elem_bits );


/* Read
 *  read from n-dimensional position
 *
 *  "dim" [ IN ] - the dimensionality of all vectors
 *
 *  "pos"  [ IN ] - n-dimensional starting position in elements
 *
 *  "buffer" [ OUT ] and "elem_count" [ IN ] - return buffer for read
 *  where "elem_count" is n-dimensional in elements
 *
 *  "num_read" [ OUT ] - n-dimensional return parameter giving back
 *      the number of read elements in every dimension
 */
KFS_EXTERN rc_t CC KArrayFileRead ( const KArrayFile *self, uint8_t dim,
    const uint64_t *pos, void *buffer, const uint64_t *elem_count,
    uint64_t *num_read );


/* Read_v
 *  read one element of variable length from n-dimensional position
 *
 *  "dim" [ IN ] - the dimensionality of all vectors
 *
 *  "pos"  [ IN ] - n-dimensional starting position in elements
 *
 *  "buffer" [ OUT ] and "elem_count" [ IN ] - return buffer for read
 *  where "elem_count" is length of buffer in elements
 *
 *  "num_read" [ OUT ] - return parameter giving back
 *      the number of read elements in every dimension
 */
KFS_EXTERN rc_t CC KArrayFileRead_v ( const KArrayFile *self, uint8_t dim,
    const uint64_t *pos, void *buffer, const uint64_t elem_count,
    uint64_t *num_read );


/* Write
 *  write into n-dimensional position
 *
 *  "dim" [ IN ] - the dimensionality of all vectors
 *
 *  "pos"  [ IN ] - n-dimensional offset where to write to
 *                   in elements
 *
 *  "buffer" [ IN ] and "elem_count" [ IN ] - data to be written
 *  where "elem_count" is n-dimensional in elements
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of elements actually written per dimension
 */
KFS_EXTERN rc_t CC KArrayFileWrite ( KArrayFile *self, uint8_t dim,
    const uint64_t *pos, const void *buffer, const uint64_t *elem_count,
    uint64_t *num_writ );


#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_arrayfile_ */
