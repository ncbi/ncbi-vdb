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

/* -----
 * A data block within the encrypted file may not divide nicely by the size of
 * its greatest alignment issue. That is there is currently a uint64_t within 
 * the KEncFileKey forcing 8-byte alignment by default but the whole of the
 * size of KEncFileKey plus the size of the KEncFileData plus the size of
 * the KEncFileCRC is not divisible by 8 so it can't be treated as a single 
 * structure for the puroses of using the sizeof() operator.
 *
 * If the size of the header, block or footer are modified in the future
 * care should be take to retain accuracy of the new versions of these
 * functions.
 */

static __inline__ uint64_t BlockId_to_EncryptedPos (uint64_t block_id)
{
    /* the whole block structure is not divisible by 8
     * so this is not a simple multiply by the sizeof KEncFileBlock
     */
    return (sizeof (KEncFileHeader) + block_id * sizeof (KEncFileBlock));
}


static __inline__ uint64_t BlockId_to_DecryptedPos (uint64_t block_id)
{
    /* the simplest conversion as its a simple multiply */
    return (block_id * sizeof (KEncFileData));
}


/* -----
 * when converting from file offsets to block ids we effectively have a
 * quotient and a remainder.  The block id is the quotient and the
 * offset within the block is the remainder.  We use lldiv not ldiv as we
 * are required to work on both 32 and 64 bit machines where long might
 * be 32 bits or it might be 64 bits.  If long long is 128 we willingly
 * sacrifice efficiency for accuracy.
 */
static __inline__
uint64_t DecryptedPos_to_BlockId (uint64_t dec_offset, uint32_t * poffset)
{
    if (poffset)
        *poffset = dec_offset % sizeof (KEncFileData);
    return dec_offset / sizeof (KEncFileData);
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


