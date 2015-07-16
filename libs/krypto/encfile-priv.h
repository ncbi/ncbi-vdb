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
 */
#ifndef _h_krypto_encfile_priv_libs_
#define _h_krypto_encfile_priv_libs_

#ifdef  __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------
 * Offset conversions between encrypted file offset, decrypted file offset
 * and block ID and block offset
 */

/* BlockId_to_CiphertextOffset
 *  converts zero-based block id to byte offset into ciphertext
 */
static __inline__
uint64_t BlockId_to_CiphertextOffset ( uint64_t block_id )
{
    return ( sizeof ( KEncFileHeader ) + block_id * sizeof ( KEncFileBlock ) );
}

/* BlockId_to_DecryptedPos
 *  converts zero-based block id to byte offset into plaintext
 */
static __inline__
uint64_t BlockId_to_PlaintextOffset ( uint64_t block_id )
{
    return block_id * sizeof ( KEncFileData );
}

/* PlaintextOffset_to_BlockId
 *  converts from byte offset into plaintext to a zero-based block id
 *  NB - will FAIL ( horribly ) if used with a plaintext SIZE
 */
static __inline__
uint64_t PlaintextOffset_to_BlockId ( uint64_t pt_offset, uint32_t * poffset )
{
    uint64_t block_id = pt_offset / sizeof ( KEncFileData );

    if ( poffset != NULL )
        * poffset = ( uint32_t ) ( pt_offset - BlockId_to_PlaintextOffset ( block_id ) );

    return block_id;
}

static __inline__
uint64_t PlaintextSize_to_BlockCount ( uint64_t pt_size, uint32_t * padding )
{
    uint64_t block_count = ( pt_size + sizeof ( KEncFileData ) - 1 ) / sizeof ( KEncFileData );

    if ( padding != NULL )
        * padding = ( uint32_t ) ( BlockId_to_PlaintextOffset ( block_count ) - pt_size );

    return block_count;
}

static __inline__
uint64_t EncryptedPos_to_BlockId (uint64_t enc_offset, uint32_t * poffset,
                                     bool * in_block)
{
    uint64_t block_id;

    if (enc_offset < sizeof (KEncFileHeader))
    {
        if (poffset)
            *poffset = 0;
        if (in_block)
            *in_block = false;
        block_id = 0;
    }
    else
    {
        uint64_t offset;

        enc_offset -= sizeof (KEncFileHeader);
        block_id = enc_offset / sizeof (KEncFileBlock);
        offset = enc_offset % sizeof (KEncFileBlock);

        if (offset <= sizeof(KEncFileKey))
        {
            if (poffset)
                *poffset = 0;
            if (in_block)
                *in_block = false;
        }
        else
        {
            offset -= sizeof(KEncFileKey);

            if (offset >= sizeof(KEncFileData))
            {
                if (poffset)
                    *poffset = 0;
                if (in_block)
                    *in_block = false;
            }
            else
            {
                if (poffset)
                    *poffset = (uint32_t)offset;
                if (in_block)
                    *in_block = true;
            }
        }
    }
    return block_id;
}

struct KFile;
struct KKey;

KRYPTO_EXTERN rc_t CC KEncFileWriteHeader (struct KFile * self);
KRYPTO_EXTERN rc_t CC KEncFileWriteHeader_v1 (struct KFile * self);
KRYPTO_EXTERN rc_t CC KEncFileWriteHeader_v2 (struct KFile * self);
KRYPTO_EXTERN rc_t CC KEncFileMakeWriteBlock (struct KFile ** pself, 
                                              struct KFile * encrypted,
                                              const struct KKey * key);

KRYPTO_EXTERN rc_t CC KEncFileMakeBlock_v2 (struct KFile ** pself,
                                         struct KFile * encrypted,
                                         const struct KKey * key);


#if 0
#if USE_UPDATE_V1
#define KEncFileWriteHeader KEncFileWriteHeader_v1
#else
#define KEncFileWriteHeader KEncFileWriteHeader_v2
#endif
#endif

#ifdef  __cplusplus
}
#endif

#endif /* #ifndef _h_krypto_encfile_priv_libs_ */


