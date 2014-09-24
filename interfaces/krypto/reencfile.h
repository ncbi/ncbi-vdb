
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

#ifndef _h_krypto_reencfile_
#define _h_krypto_reencfile_

#ifndef _h_krypto_extern_
#include <krypto/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Currently only the read version is implmented
 */
#define REENCFILE_WRITE_SUPPORTED 0
/*
 * Currently the input encrypted KFile must respond to KFileSize
 * with the size of the encrypted file. 
 */
#define REENCFILE_STREAM_SUPPORTED 0


struct KFile;
struct KKey;

typedef struct KReencFile KReencFile;

/* ----------
 * Read mode is fully seekable if the underlying KFile is seekable some
 * integrity checking will not be performed in allowing this seeking.
 */
KRYPTO_EXTERN rc_t CC KReencFileMakeRead (const struct KFile ** pself, 
                                          const struct KFile * encrypted,
                                          const struct KKey * deckey,
                                          const struct KKey * enckey);

KRYPTO_EXTERN rc_t CC KEncryptFileMakeRead (const struct KFile ** pself, 
                                            const struct KFile * encrypted,
                                            const struct KKey * enckey);


/* ----------
 * Write mode encrypted file can only be written straight through form the
 * first byte to the last.
 */
#if REENCFILE_WRITE_SUPPORTED
KRYPTO_EXTERN rc_t CC KReencFileMakeWrite (struct KFile ** pself, 
                                           struct KFile * encrypted,
                                           const struct KKey * deckey,
                                           const struct KKey * enckey);
#endif




#ifdef __cplusplus
}
#endif

#endif /* _h_krypto_reencfile_ */
