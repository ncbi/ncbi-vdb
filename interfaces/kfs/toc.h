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

#ifndef _h_kfs_toc_
#define _h_kfs_toc_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_kfs_directory_
#include <kfs/directory.h>
#endif

#ifndef _h_klib_pbstree_
#include <klib/pbstree.h>
#endif

#ifndef _h_kfs_sra_
#include <kfs/sra.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

/*==========================================================================
 * The TOC (Table of Contents) is the basis of non-O/S level file systems
 * directory structures.
 *
 * The TOC is based on a set of entries.  An entry can represent a file
 * (collection of bytes), a directory (a list of entries, or a link (an
 * entry that refers to another entry).
 *
 * Files are represented either as a contiguous array of bytes or a
 * sequence of smaller chunks of contiguous bytes.  Other than the initial
 * creation of the chunked version, most operations on the chunked file
 * will be able to treat it as a contiguous sequence with any gaps between
 * contiguous chunks being treated as if they were all zero bytes.  So
 * for the most part the two types of file are the same to the outside
 * world.
 *
 * A directory is viewed as a sorted list of named references to files.
 * In the first versions the form is of a binary search tree per directory
 * but for the most part that detail is irrelevant.  In the tree a directory
 * is an entry that contains its own tree.  A path is a sequence of named
 * entries in order - the representation of a path is the Unix/Posix style
 * path.
 *
 * A link is either a hard or soft link 9using Unix/Posix terminology. A
 * soft link is a named item that contains a path that names another item.
 * A hard link will as much as possible refer directly to the same file,
 * directory or another link as another item.
 *
 * Each entry is represented by a TocEntry.
 *
 * A path as mentioned above is a list of facets (names of directories) up to
 * a final leaf item that can be any of the item types.
 *
 * Position is the logical offset within a file or a particualr byte,  The
 * first byte is at position 0.
 */


/*--------------------------------------------------------------------------
 * forwards
 */
struct BSTree;
struct Vector;
struct String;
struct KFile;


/*--------------------------------------------------------------------------
 * KTocChunk
 *  Describes a chunk of valid data within a file.  The concept is that a file
 *  can be made up of discrete chunks that might not be consecutive on the 
 *  storage system.  There might also be gaps between the chunks - gaps that 
 *  read as all zeroes.  These could be tarred up versions of sparse files
 *  where the only things stored on disk are small regions of non-zero data. 
 *  Or it could be some archival format where data is broken into some form of
 *  packets.
 *
 *  64 bit values are used for file sizes up to 18,446,744,073,709,551,617 bytes
 *  up to exabytes.  The human genome is in the gigabyte range so 32 bit values
 *  are too close to the limits so the larger range was used.
 *
 *  This struct is very clear and public so chunks can be built up or used  freely
 *  by non-file-system specific code.
 */
typedef struct KTocChunk KTocChunk;
struct KTocChunk
{
    /* position of chunk within logical file */
    uint64_t logical_position;

    /* position of chunk within source container file */
    uint64_t source_position;

    /* chunk size */
    uint64_t size;
};



KFS_EXTERN rc_t CC KDirectoryOpenTocFileRead( const KDirectory * self,
                                              struct KFile const ** pfile,
                                              KSRAFileAlignment align,
                                              bool (CC * filter) (const KDirectory*,const char*,void*),
                                              void * filter_param,
                                              rc_t (CC * usort)(const KDirectory*, struct Vector*) );




#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_toc_ */
