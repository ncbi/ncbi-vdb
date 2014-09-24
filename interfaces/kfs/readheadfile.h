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

#ifndef _h_kfs_readheadfile_
#define _h_kfs_readheadfile_

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
 * forwards allowing opaque class typing
 */
struct KFile;


/* MakeReadHead
 *  make a read-only file where we buffer only the first part of the file
 *  this is pretty much only needed to type a file when we don't know
 *  or want to see if we can acess randomly.  This is a fairly smart type
 *  that will take ownership of the pointer to the 'original' KFile
 *  reference.  If it finds that the 'original' KFile is seekable (has
 *  random access) it will merely return a reference to that const KFile *
 *  rather than creating a new KFile that will in most cases pass through 
 *  all method calls to the original.
 *
 *  This type was created primarily to allow the reading of the first part
 *  of a file for type recognition with out forcing numerous cases of
 *  duplicated code to check for random access on an existing KFile reference
 *  and handling of the pre-read in the application.
 *
 *  This class is intended for files that once typed will be read straight
 *  through.  If more random access to the file is needed the KBufFile is 
 *  indicated rather than this one.  If a straight through access is needed
 *  this is a far lighter weight solution than KBufFile.
 *
 *  A references is added to the original even if it becomes used as the new
 *  file because it allows random access.
 *
 *  "buf" [ OUT ] - return parameter for new buffered file
 *
 *  "original" [ IN ] - source file to be buffered. must have read access
 *
 *  "bsize" [ IN ] - buffer size
 */
KFS_EXTERN 
rc_t CC KFileMakeReadHead (const struct KFile ** pself,
                           const struct KFile * original,
                           size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_readheadfile_ */
