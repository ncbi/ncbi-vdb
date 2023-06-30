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

#ifndef _h_klib_checksum_
#define _h_klib_checksum_

#ifndef _h_klib_extern_
#include <klib/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * CRC32
 */

/* CRC32Init
 *  initializes table
 *  IDEMPOTENT
 */
KLIB_EXTERN void CC CRC32Init ( void );

/* CRC32
 *  runs checksum on arbitrary data, returning result
 *  initial checksum to be passed in is 0
 *  subsequent checksums should be return from prior invocation
 */
KLIB_EXTERN uint32_t CC CRC32 ( uint32_t checksum, const void *data, size_t size );


/*--------------------------------------------------------------------------
 * MD5
 *  taken from Peter Deutsch's sources due to simplicity
 */
typedef struct MD5State MD5State;
struct MD5State
{
    uint32_t count [ 2 ];
    uint32_t abcd [ 4 ];
    uint8_t buf [ 64 ];
};

/* Init
 *  initialize the algorithm and state block
 */
KLIB_EXTERN void CC MD5StateInit ( MD5State *md5 );

/* Append
 *  run MD5 on data block
 *  accumulate results into "md5"
 *  processes data in chunks
 */
KLIB_EXTERN void CC MD5StateAppend ( MD5State *md5, const void *data, size_t size );

/* Finish
 *  processes any remaining data in "md5"
 *  returns 16 bytes of digest
 */
KLIB_EXTERN void CC MD5StateFinish ( MD5State *md5, uint8_t digest [ 16 ] );


/*--------------------------------------------------------------------------
 * SHA
 */
typedef struct SHA32bitState SHA1State, SHA256State;
struct SHA32bitState
{
    uint64_t len;
    uint32_t H [ 8 ];
    uint32_t cur;
    uint8_t  W [ 64 ];
};

typedef struct SHA64bitState SHA384State, SHA512State;
struct SHA64bitState
{
    uint64_t len;
    uint64_t H [ 8 ];
    unsigned long cur;
    uint8_t W [ 128 ];
};


/* Init
 *  initialize the algorithm and state block
 */
KLIB_EXTERN void CC SHA1StateInit ( SHA1State *state );
KLIB_EXTERN void CC SHA256StateInit ( SHA256State *state );
KLIB_EXTERN void CC SHA384StateInit ( SHA384State *state );
KLIB_EXTERN void CC SHA512StateInit ( SHA512State *state );


/* Append
 *  run SHA-x on data block
 *  accumulate results into "state"
 *  processes data in chunks
 */
KLIB_EXTERN void CC SHA1StateAppend ( SHA1State *state, const void *data, size_t size );
KLIB_EXTERN void CC SHA256StateAppend ( SHA256State *state, const void *data, size_t size );
KLIB_EXTERN void CC SHA384StateAppend ( SHA384State *state, const void *data, size_t size );
KLIB_EXTERN void CC SHA512StateAppend ( SHA512State *state, const void *data, size_t size );


/* Finish
 *  processes any remaining data in "state"
 *  returns N bytes of digest
 *  N = 20 for SHA-1
 *  N = 32 for SHA-256
 *  N = 48 for SHA-384
 *  N = 64 for SHA-512
 */
KLIB_EXTERN void CC SHA1StateFinish ( SHA1State *state, uint8_t digest [ 20 ] );
KLIB_EXTERN void CC SHA256StateFinish ( SHA256State *state, uint8_t digest [ 32 ] );
KLIB_EXTERN void CC SHA384StateFinish ( SHA384State *state, uint8_t digest [ 48 ] );
KLIB_EXTERN void CC SHA512StateFinish ( SHA512State *state, uint8_t digest [ 64 ] );


#ifdef __cplusplus
}
#endif

#endif /* _h_klib_checksum_ */
