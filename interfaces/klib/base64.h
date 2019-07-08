/*==============================================================================
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

#ifndef _h_klib_base64_
#define _h_klib_base64_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct String;
struct KDataBuffer;

/* encodeBase64
 *  encode a buffer of binary data as string of base64 ASCII characters
 *
 *  "encoded" [ OUT ] - base64 encoded string representing input data
 *  must be freed with StringWhack() [ see <klib/text.h> ]
 *
 *  "data" [ IN ] and "bytes" [ IN ] - buffer of binary data to be encoded
 */
KLIB_EXTERN rc_t CC encodeBase64 ( struct String const ** encoded, const void * data, size_t bytes );


/* decodeBase64
 *  decode a string of base64 ASCII characters into a buffer of binary data
 *
 *  "decoded" [ OUT ] - pointer to an UNINITIALIZED KDataBuffer structure that
 *  will be initialized by the function to contain decoded binary data. must be
 *  freed with KDataBufferWhack() [ see <klib/data-buffer.h> ]
 *
 *  "encoding" [ IN ] - base64-encoded text representation of data
 */
KLIB_EXTERN rc_t CC decodeBase64 ( struct KDataBuffer * decoded, struct String const * encoding );


/* encodeBase64URL
 * decodeBase64URL
 *  identical to simple base-64 codec functions above,
 *  except that base-64-URL encoding is used.
 */
KLIB_EXTERN rc_t CC encodeBase64URL ( struct String const ** encoded, const void * data, size_t bytes );
KLIB_EXTERN rc_t CC decodeBase64URL ( struct KDataBuffer * decoded, struct String const * encoding );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_base64_ */
