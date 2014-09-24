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

#ifndef _h_krypto_wgaencryptfile_
#define _h_krypto_wgaencryptfile_

#ifndef _h_krypto_extern_
#include <krypto/extern.h>
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
 * KFile
 *  wga extensions
 */

/* MakeWGAEncRead
 *
 * Parameters:
 *   pself is a KFile that will represent the unencrypted side of operations.
 *
 *   encrypted is a KFile that will represent the encrypted side of operations.
 *
 *   key is an arbitrary array of characters
 *
 *   key_size is how many bytes to use from key.  Maximum used is 256.
 *
 *   encoding is one of the three values defined by FER_ENCODING above.  Any
 *   other values will cause a failure.
 *
 * KFileMakeWGAEncRead
 *   A file opened for read can only read and decrypt an encrypted file.  Seeks
 *   within the file are allowed if the KFile for the encrypted file allows
 *    seeks.
 * 
 * KFileMakeWGAEncUpdate
 *   A file opened for update must be able to read and write from the encrypted
 *   file.  Seeks within the file are allowed if the KFile for the encrypted 
 *   file allows seeks.
 *
 * KFileMakeWGAEncRead
 *   A Write opened file can only be written.  Seeks backwards are not allowed.
 *   Seeks forward will cause NUL data bytes to be inserted before encryption.
 *   An attempt will be made to truncate the encrypted file but failure of that
 *   operation will not fail this call.  Writes will begin at position 0.  If
 *   the file already existed and could not be truncated a corrupt file will
 *   result if the whole of the old contents are not over-written.
 *
 *
 * NOTE: cipher must be of type kcipherAES
 *
 */
KRYPTO_EXTERN rc_t CC KFileMakeWGAEncRead (const struct KFile ** pself, 
                                           const struct KFile * encrypted,
                                           const char * key,
                                           size_t key_size);


/* IsWGAEnc
 *  identify whether a file is a KFileWGAEnc type encrypted file by the header.
 *  read the header into a buffer and pass it into this function.  
 *  The buffer_size needs to be at least 8 but more bytes lead to a better
 *  check up to the size of the header of a KFileWGAEnc type encrypted file.
 *  This file type is deprecated and is not versioned.  Hopefully we won't
 *  have to deal with changes to the format.
 *
 * Possible returns:
 *  0:
 *      the file is an identified KFileWGAEnc type file.  False positives are
 *      possible if a file happens to match at 8 or more bytes
 *
 *  RC (rcFS, rcFile, rcIdentifying, rcFile, rcWrongType)
 *      the file is definitely not a KFileWGAEnc type encrypted file.
 *     
 *  RC (rcFS, rcFile, rcIdentifying, rcParam, rcNull)
 *      bad parameters in the call
 *
 *  RC (rcFS, rcFile, rcIdentifying, rcBuffer, rcInsufficient)
 *      not a large enough buffer to make an identification
 */
KRYPTO_EXTERN rc_t CC KFileIsWGAEnc (const void * buffer, size_t buffer_size);

KRYPTO_EXTERN rc_t CC WGAEncValidate (const struct KFile * file, 
                                      const char * password,
                                      size_t password_size);

#ifdef __cplusplus
}
#endif

#endif /* _h_kfs_wgaencryptfile_ */
