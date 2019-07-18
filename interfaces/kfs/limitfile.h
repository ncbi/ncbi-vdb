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

#ifndef _h_kfs_limitfile_
#define _h_kfs_limitfile_

#ifndef _h_kfs_extern_
#include <kfs/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct KFile;

/*--------------------------------------------------------------------------
 * MakeLimitFile
 *  a file that limits reads or writes to being within a particular block
 *
 *  "obj" [ OUT ] - return parameter for newly created limit file
 *
 *  "original" [ IN ] - file to be wrapped.
 *   on success, will have a new reference created to it.
 *
 *  "block_size" [ IN ] - the size of the emulated I/O block. MUST be
 *   an even power of two, or the function will fail.
 */
KFS_EXTERN rc_t CC KFileMakeLimitFile ( struct KFile ** obj,
    struct KFile const * original, size_t block_size );

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_limitfile_ */
