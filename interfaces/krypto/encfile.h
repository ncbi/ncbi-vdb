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

/*
 * This needs to be changed to true in about the middle of summer 2013
 *
 * When re-enabling this feature also fix test/kreypto/Makefile
 */
#define SENC_IS_NENC_FOR_WRITER 0


#ifndef _h_krypto_encfile_
#define _h_krypto_encfile_

#ifndef _h_krypto_extern_
#include <krypto/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct KFile;
struct KKey;

typedef struct KEncFile KEncFile;

/* ----------
 * Read mode is fully seekable if the underlying KFile is seekable some
 * integrity checking will not be performed in allowing this seeking.
 *
 * One difference between update and read mode is the handling of
 * "missing blocks". Open for update will read a missing block as all
 * zero while 
 */
KRYPTO_EXTERN rc_t CC KEncFileMakeRead (const struct KFile ** pself,
                                        const struct KFile * encrypted_input,
                                        const struct KKey * key);


/* ----------
 * Write mode encrypted file can only be written straight through from the
 * first byte to the last.
 */
KRYPTO_EXTERN rc_t CC KEncFileMakeWrite (struct KFile ** pself,
                                         struct KFile * encrypted_output,
                                         const struct KKey * key);


/* ----------
 * Update mode is read/write mode where seeking within the file is allowed.
 * 
 * One difference between update and read mode is the handling of
 * "missing blocks". Open for update will read a missing block as all
 * zero while 
 */
KRYPTO_EXTERN rc_t CC KEncFileMakeUpdate (struct KFile ** pself, 
                                          struct KFile * encrypted,
                                          const struct KKey * key);


/* ----------
 * Validate mode can not be read or written.
 * Upon open the whole file is read from begining to end and all CRC
 * and other integrity checks are performed immedaitely
 */
KRYPTO_EXTERN rc_t CC KEncFileValidate (const struct KFile * encrypted);


/* ----------
 * Identify whether a file is a KEncFile type encrypted file by the header.
 * read the header into a buffer and pass it into this function.  
 * The buffer_size needs to be at least 8 but more bytes lead to a better
 * check up to the size of the header of a KEncFile type encrypted file.
 * As the header may change in the future (in a backwards compatible way)
 * that size might change from the current 16.
 *
 * Possible returns:
 * 0:
 *      the file is an identified KEncFile type file.  False positives are
 *      possible if a file happens to match at 8 or more bytes
 *
 * RC (rcFS, rcFile, rcIdentifying, rcFile, rcWrongType)
 *      the file is definitely not a KEncFIle type encrypted file.
 *     
 * RC (rcFS, rcFile, rcIdentifying, rcParam, rcNull)
 *      bad parameters in the call
 *
 * RC (rcFS, rcFile, rcIdentifying, rcBuffer, rcInsufficient)
 *      not a large enough buffer to make an identification
 */

/* requires NCBInenc or NCBIsenc or signature but if available
 * checks the byte order and version fields
 */
KRYPTO_EXTERN rc_t CC KFileIsEnc (const char * buffer, size_t buffer_size);

/* same as above but requires NCBIsenc signature only */
KRYPTO_EXTERN rc_t CC KFileIsSraEnc (const char * buffer, size_t buffer_size);



#ifdef __cplusplus
}
#endif

#endif /* _h_krypto_encfile_ */
