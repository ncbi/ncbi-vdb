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
#include <krypto/extern.h>
#include <krypto/encfile.h>
#include <krypto/encfile-priv.h>
#include "encfile-priv.h"
#include <krypto/cipher.h>
#include <krypto/ciphermgr.h>
#include <krypto/key.h>

/* #include "aes-priv.h" */

#include <klib/rc.h>
#include <klib/checksum.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/debug.h>
#include <klib/status.h>
/* #include <klib/vector.h> */
/* #include <klib/status.h> */
#include <kfs/file.h>
#include <kfs/sra.h>
#include <sysalloc.h>

#include <byteswap.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* #include <klib/out.h> */


/* ----------------------------------------------------------------------
 * KEncFile
 *   Base object class for the encryption, decryption and validation of
 *   the file format defined above
 */

#define KFILE_IMPL struct KEncFile
#include <kfs/impl.h>


#include "encfile-priv.h"

typedef struct KEncFileCiphers KEncFileCiphers;
struct KEncFileCiphers
{
    KCipher * master;
    KCipher * block;
};


typedef struct KEncFileIVec { uint8_t ivec [16]; } KEncFileIVec;

/* -----
 */
struct KEncFile
{
    KFile dad;                  /* base class */
    KFile * encrypted;          /* encrypted file as a KFile */
    KEncFileCiphers ciphers;    /* file and block ciphers */
    KEncFileBlock block;        /* current data block */
    KEncFileFooter foot;        /* contains crc checksum and block count */
    uint64_t dec_size;          /* size of decrypted file */
    uint64_t enc_size;          /* size of encrypted file */
    bool dirty;                 /* data written but not flushed set in Write cleared in Flush*/
    bool seekable;              /* we can seek within the encrypted file */
    bool size_known;            /* can we know the size? Only streaming read can not know */
    bool bswap;                 /* file created on system of opposite endianess */
    bool changed;               /* some write has happened cleared in Make, set in BufferWrite */
    bool sought;                /* did a seek on a read or write invalidating crc checksum */
    bool has_header;            /* have we read or written a header? */
    bool eof;                   
    bool sra;                   /* we know we are encrypting an SRA/KAR archive file */
    bool swarm;                 /* block mode for swarm mode using KReencFile or KEncryptFile */
    KEncFileVersion version;    /* version from the header if read; or the one being written */
};


/* ----------
 * BufferCalcMD5
 *    Generate the MD5 digest for a buffer
 */
static __inline__
void BufferCalcMD5 (const void * buffer, size_t size, uint8_t digest [16])
{
    MD5State state;

    assert (buffer);
    assert (size);
    assert (digest);

    MD5StateInit (&state);
    MD5StateAppend (&state, buffer, size);
    MD5StateFinish (&state, digest);
}


/* -----
 * return true or false as to whether the the buffer described is all 0 bits
 */
static __inline__
bool BufferAllZero (const void * buffer_, size_t size)
{
    const uint8_t * buffer;
    size_t count;
    bool ret;

    assert (buffer_);

    buffer = buffer_;
    ret = true;

    for (count = 0; count < size; ++count)
    {
        if (buffer[count] != '\0')
        {
            ret = false;
            break;
        }
    }
    return ret;
}

/* Compensate that KFileReadAll became non-blocking.
   Keep calling KFileReadAll until buffer fills up
   to satisfy caller's expectations */
static rc_t KFile_ReadAllBlocking(const KFile *self, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read)
{
    size_t count = 0;
    rc_t rc = KFileReadAll(self, pos, buffer, bsize, &count);
    size_t total = count;

    if (rc == 0 && count != 0 && count < bsize) {
        uint8_t *b = buffer;
        for (b = buffer; total < bsize; total += count) {
            count = 0;
            rc = KFileReadAll(self, pos + total, b + total, bsize - total,
                &count);
            if (rc != 0 || count == 0)
                break;
        }
    }

    if (total != 0) {
        *num_read = total;
        return 0;
    }

    return rc;
}

/* ----------
 * BufferRead
 *    Read from an encrypted file and update size if warranted
 *
 * Read's aren't guaranteed to get full amount
 */
static
rc_t KEncFileBufferRead (KEncFile * self, uint64_t offset, void * buffer,
                         size_t bsize, size_t * num_read)
{
    rc_t rc;

    assert (self);
    assert (self->encrypted);
    assert (buffer);
    assert (bsize > 0);
    assert (num_read);

    rc = KFile_ReadAllBlocking (self->encrypted, offset, buffer, bsize,
        num_read);
    if (rc == 0)
    {
        if (self->enc_size < offset + *num_read)
        {
            self->enc_size = offset + *num_read;
        }
    }
/* leave logging to callers? */
    else
        PLOGERR (klogErr, (klogErr, rc, "Error reading from encrypted file "
                           "at '$(P)", "P=%lu", offset));

    return rc;
}


/* ----------
 * BufferWrite
 *    write to an encrypted file, mark it as changed and update size if warranted
 */
static
rc_t KEncFileBufferWrite (KEncFile * self, uint64_t offset, const void * buffer,
                          size_t bsize, size_t * num_writ)
{
    rc_t rc = 0;

    assert (self);
    assert (self->encrypted);
    assert (buffer);
    assert (bsize > 0);
    assert (num_writ);

    rc = KFileWriteAll (self->encrypted, offset, buffer, bsize, num_writ);
    if (rc != 0)
    {
/* leave logging to callers? */
        PLOGERR (klogErr, (klogErr, rc, "Error writing to encrypted file "
                           "at '$(P)", "P=%lu", offset));
        // make sure we don't crash later in destructor
        if (self->enc_size > offset + *num_writ)
        {
            self->enc_size = offset + *num_writ;
        }
        return rc;
    }
    
    if (bsize != *num_writ)
    {
/*         KOutMsg ("%s: bsize %zu *num_writ %zu\n",__func__, bsize, *num_writ); */
        rc = RC (rcKrypto, rcFile, rcWriting, rcTransfer, rcIncomplete);
/* leave logging to callers? */
        PLOGERR (klogErr, (klogErr, rc, "Error writing full buffer to"
                           " encrypted file at '$(P) wrote '$(W)'",
                           "P=%lu,W=%zu", offset, *num_writ));
    }
    else if (*num_writ > 0)
    {
        self->changed = true;
        if (self->enc_size < offset + *num_writ)
        {
            self->enc_size = offset + *num_writ;
        }
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * operations on KEncFileHeader
 *
 * The header only needs to be read and validated or written  we need not 
 * reatian within the KEncFile object any information about the header in
 * the initial version of this file format.
 */

/* -----
 * the first eight bytes of the file are two four byte strings
 *  The first is a common "NCBI"
 *  The second is the format specific "nenc"
 */
#if 0
static const KEncFileSig KEncFileSignature = "NCBInenc";
#endif

/* -----
 * the common constant used throughout the project to check the byte order 
 * as written by the system which created the file
 */
/* enum fails to handle these due to integer overflow */

#define eEncFileByteOrderTag     (0x05031988)
#define eEncFileByteOrderReverse (0x88190305)

/* ----
 */
#define eCurrentVersion        (0x00000002)
#define eCurrentVersionReverse (0x02000000)


static
const KEncFileHeader const_header
= { "NCBInenc", eEncFileByteOrderTag, eCurrentVersion };


static
const KEncFileHeader const_bswap_header
= { "NCBInenc", eEncFileByteOrderReverse, eCurrentVersionReverse };

/* skipping v1 for NCBIkenc */
static
const KEncFileHeader const_header_sra
= { "NCBIsenc", eEncFileByteOrderTag, eCurrentVersion };


#if 0
static
const KEncFileHeader const_bswap_header_sra
= { "NCBIsenc", eEncFileByteOrderReverse, eCurrentVersionReverse };
#endif
    
/* ----------
 * HeaderRead
 *    Read the header of an encrypted file and validate it.
 *
 * We only allow a missing header for an empty file opened for update
 */
static
rc_t KEncFileHeaderRead (KEncFile * self)
{
    KEncFileHeader header;
    size_t num_read;
    rc_t rc;

    assert (self);

    rc = KEncFileBufferRead (self, 0, &header, sizeof (header), &num_read);
    if (rc)
        LOGERR (klogErr, rc, "error reading encrypted file header");

    else if ((num_read == 0) && (self->dad.write_enabled))
    {
        /* only allow read to fail with zero length if opened with write */
        self->version = eCurrentVersion;
        self->bswap = false;
        self->enc_size = 0; /* redundant? */
        self->dec_size = 0; /* redundant? */
    }
    else if (num_read != sizeof (header))
    {
        rc = RC (rcFS, rcFile, rcConstructing, rcHeader, rcTooShort);
        PLOGERR (klogErr,
                 (klogErr, rc, "error reading full header of encrypted "
                  "file  wanted '$(S)' got '$(N); bytes read", "S=%u,N=%u",
                  sizeof (header), num_read));
    }
    else
    {
        rc_t orc;

        if (memcmp (header.file_sig, const_header.file_sig,
                     sizeof (header.file_sig)) == 0)
            self->sra = false;
        else if (memcmp (header.file_sig, const_header_sra.file_sig,
                         sizeof (header.file_sig)) == 0)
            self->sra = true;
        else
        {
            rc = RC (rcFS, rcFile, rcConstructing, rcHeader, rcInvalid);
            LOGERR (klogErr, rc, "file signature not correct for encrypted file");
        }

        switch (header.byte_order)
        {
        case eEncFileByteOrderTag:
            self->bswap = false;
            self->version = header.version;
            break;

        case eEncFileByteOrderReverse:
            self->bswap = true;

            /* assert for the right bswap call */
            assert (sizeof (self->version) == 4);

            self->version = bswap_32 (header.version);
            break;

        default:
            orc = RC (rcFS, rcFile, rcConstructing, rcByteOrder, rcInvalid);
            PLOGERR (klogErr, (klogErr, rc, "invalid byte order flag '$(F); in "
                               "encrypted file header", "F=%X",
                               header.byte_order));
            if (rc == 0)
                rc = orc;
            break;
        }

        if ((self->version > eCurrentVersion) || (self->version == 0))
        {
            orc = RC (rcFS, rcFile, rcConstructing, rcHeader, rcBadVersion);
            PLOGERR (klogErr, (klogErr, orc, "can not decrypt version '$(V)'",
                               "V=%u", header.version));
            if (rc == 0)
                rc = orc;
        }
        if (rc == 0)
            self->has_header = true;
    }
    return rc;
}


/* -----
 * HeaderWrite
 */
#ifndef SENC_IS_NENC_FOR_WRITER
#define SENC_IS_NENC_FOR_WRITER 1
#endif

static
rc_t KEncFileHeaderWrite (KEncFile * self)
{
    rc_t rc;
    size_t num_writ;
    const KEncFileHeader * head;

#if SENC_IS_NENC_FOR_WRITER
    head = self->sra 
        ? (self->bswap ? &const_bswap_header_sra : &const_header_sra)
        : (self->bswap ? &const_bswap_header : &const_header);
#else
    head = self->bswap ? &const_bswap_header : &const_header;
#endif

    rc = KEncFileBufferWrite (self, 0, head, sizeof * head, &num_writ);
    if (rc)
        LOGERR (klogErr, rc, "Failed to write encrypted file header");

    else
        /* forced upgrade by writing the header */
        self->version = eCurrentVersion;

    if (rc == 0)
        self->has_header = true;

    return rc;
}

LIB_EXPORT rc_t CC KEncFileWriteHeader_v2  (KFile * self)
{
    if (self == NULL)
        return RC (rcKrypto, rcFile, rcWriting, rcSelf, rcNull);

    return KEncFileHeaderWrite ((KEncFile*)self);
}



/* ----------------------------------------------------------------------
 * operations on KEncFileFooter
 */

/* ----------
 * Validate
 * we expect to read a Footer when we expect a Block so we validate in RAM
 * without a Read
 *
 * we're just comparing the values in the footer against expected values
 */
static
rc_t KEncFileFooterValidate (const KEncFile * self, 
                             const uint64_t block_count,
                             const uint64_t crc_checksum)
{
    rc_t rc1 = 0, rc2 = 0;

    assert (self);

    if (self->foot.block_count != block_count)
    {
        rc1 = RC (rcKrypto, rcFile, rcValidating, rcFile, rcCorrupt);
        LOGERR (klogErr, rc1, "bad block count in encrypted file footer");
    }
    /*
     * crcs should match or the crc should be 0 and the version is 1
     */
    if ((self->foot.crc_checksum != crc_checksum) &&
        ((self->version == 1) || (crc_checksum != 0)))
    {
        rc2 = RC (rcKrypto, rcFile, rcValidating, rcChecksum, rcCorrupt);
        LOGERR (klogErr, rc2, "bad crc checksum in encrypted file footer");
    }
    return (rc1 ? rc1 : rc2);
}


static __inline__
void KEncFileFooterSwap (const KEncFile * self, KEncFileFooter * foot)
{
    assert (self);
    assert (foot);

    if (self->bswap)
    {
        assert (sizeof (foot->block_count) == 8);
        foot->block_count = bswap_64 (foot->block_count);

        assert (sizeof (foot->crc_checksum) == 8);
        foot->crc_checksum = bswap_64 (foot->crc_checksum);
    }
}


/* ----------
 * Read
 * If we know where the footer is we can read it specifically
 */
#if 0
static
rc_t KEncFileFooterRead (KEncFile * self, const uint64_t pos,
                         const bool validate)
{
    union foot_plus
    {
        KEncFileFooter foot;
        uint8_t bytes [sizeof (KEncFileFooter) + 1];
    } u;
    size_t num_read;
    rc_t rc;

    assert ((self->size_known == true) || (self->size_known == false));
    assert (((self->size_known == true) &&
             ((pos + sizeof (u.foot)) == self->enc_size)) ||
            (pos == self->enc_size));

    /* try to read too much just to validate nothing after the footer */
    rc = KEncFileBufferRead (self, pos, u.bytes, sizeof u.bytes, &num_read);
    if (rc)
        PLOGERR (klogErr, (klogErr, rc, "failed to read encrypted file footer "
                           "at '$(P)'", "P=%lu", pos));

    else if (num_read == sizeof u.foot)
    {
        KEncFileFooterSwap (self, &u.foot);

        if (validate)
            rc = KEncFileFooterValidate (self, u.foot.block_count,
                                         u.foot.crc_checksum);

        if (rc == 0)
        {
            self->foot.block_count = u.foot.block_count;
            self->foot.crc_checksum = u.foot.crc_checksum;
        }
    }
    else if (num_read < sizeof u.foot)
        rc = RC (rcKrypto, rcFile, rcReading, rcSize, rcInsufficient);

    else
    {
        assert (num_read > sizeof u.foot);
        rc = RC (rcKrypto, rcFile, rcReading, rcSize, rcExcessive);
    }

    return rc;
}
#endif

/* ----------
 * Write
 * when we write a footer we write from the values in the object.  They
 * are stored in the same object format as the footer so its very simple
 */
static
rc_t KEncFileFooterWrite (KEncFile * self)
{
    KEncFileFooter foot;
    uint64_t pos;
    size_t num_writ;
    rc_t rc;

    if (self->sought)
    {
        self -> foot . block_count = foot . block_count =
            PlaintextSize_to_BlockCount ( self -> dec_size, NULL );
        foot . crc_checksum = 0;
    }
    else
    {
        memmove ( & foot, & self -> foot, sizeof foot );
    }

    KEncFileFooterSwap (self, &foot);

    if (self->sought)
        foot.crc_checksum = 0;

    pos = BlockId_to_CiphertextOffset ( self -> foot . block_count );

    assert ((self->size_known == true) || (self->size_known == false));

    rc = KEncFileBufferWrite (self, pos, &foot, sizeof (foot),
                              &num_writ);
    if (rc == 0)
    {
        if (num_writ != sizeof (foot))
        {
            rc = RC (rcFS, rcFile, rcWriting, rcFile, rcInsufficient);
            LOGERR (klogErr, rc, "faled to write correctly sized fotter "
                    "for encrypted file");
        }
        else
            assert ((pos + sizeof (foot)) == self->enc_size);
    }
    return rc;
}


/* ----------
 * IvecInit
 *    create the ivec for a given block
 *    done in a function to ensure decrypt and encrypt use the same code to
 *    generate this.  Anything used to create this has to be available to
 *    code that doesn't know the content of the data or the state of the file
 *    beyond the location of the block with in the file.
 *
 *    This is definitely over-kill using the MD5.
 */
static __inline__
void KEncFileIVecInit (const uint64_t block_id, KEncFileIVec * ivec)
{
    BufferCalcMD5 (&block_id, sizeof block_id, ivec->ivec);
}

 
/* ----------
 * BlockEncrypt
 *
 * Not thread safe - use of cipher schedules ivec and block key in the ciphers
 *
 * If this function were a protected region where only one thread could be in
 * the body of this function at a time it could be made thread safe.
 */
static
rc_t KEncFileBlockEncrypt (KEncFile * self, KEncFileBlock * d,
                           KEncFileBlock * e)
{
    SHA256State state;
    uint64_t id;
    uint16_t valid;
    uint16_t saved_valid;
    KEncFileCRC crc;
    KEncFileIVec ivec;
    rc_t rc;

    assert (self);
    assert (d);
    assert (e);

    /*
     * First we finish preparing the two ciphers by creating the block
     * user key out of the first part of the data and the shared Initialization
     * vector for Chained Block Cipher mode encryption out of the block id
     * 
     * create the initialization vector for this block
     */
    KEncFileIVecInit (d->id, &ivec);

    /*
     * set the ivec for both the master and data block ciphers
     */
    rc = KCipherSetEncryptIVec (self->ciphers.master, &ivec);
    if (rc)
        return rc;

    rc = KCipherSetEncryptIVec (self->ciphers.block, &ivec);
    if (rc)
        return rc;

    /*
     * create the block user key out of the first 4kb of data and the block id 
     */
    saved_valid = valid = d->u.valid;
    id = d->id;

    SHA256StateInit (&state);
    SHA256StateAppend (&state, d->data, 
                       valid > 4096 ? 4096 : valid);
    SHA256StateAppend (&state, &id, sizeof (id));
    SHA256StateFinish (&state, d->key);
        
    /*
     * create the block key schedule out of the block user key
     */
    rc = KCipherSetEncryptKey (self->ciphers.block, d->key, sizeof d->key);
    if (rc)
        return rc;

    /* 
     * Salt the block using the randomish user key to randomly select
     * data from the valid data.
     *
     * This will cover the data portion of the block past the last valid
     * byte.
     *
     * NOTE we are accessing a byte array as a word array. COuld be trouble
     *      on some archaic processors such as the MC68000 family.
     *
     * NOTE we are using the array named data to access data beyond it's end
     *      based on knowledge of the structure of the KEncFileBlock.
     */
    {
        uint16_t * pw;
        unsigned int windex;
        unsigned int rindex;
        size_t bindex;

        pw = (uint16_t*)d->key;
        windex = 0;

        for (bindex = valid;
             bindex < sizeof d->data + sizeof d->u; 
             ++ bindex)
        {
            /* this goes beyond the end of the data array by design */
            rindex = (size_t)pw[windex];
            rindex %= bindex;

            d->data[bindex] = d->data[rindex];
        
            ++rindex;
            if (rindex >= sizeof self->block.key / sizeof *pw)
                rindex = 0;
        }
    }

    /* 
     * If we are modifying a block created on a system with a different default
     * Endian choice we'll need to byte swap the block id and the block valid
     * count
     */
    if (self->bswap)
    {
        assert (sizeof id == 8);
        id = bswap_64 (id);

        assert (sizeof (valid = 2));
        valid = bswap_16 (valid);
    }

    /* is this a bswap problem? */
    if (saved_valid == sizeof d->data)
        d->u.valid |= valid;
    else
        d->u.valid = valid;

    e->id = id;

    /* 
     * encrypt the block user key into the buffer
     */
#if 0
    {
        unsigned iii;
        KOutMsg ("v2 decrypted key %lu\n", d->id);
        for (iii = 0; iii < sizeof (d->key); +++ iii) 
            KOutMsg ("%2.2x ", d->key[iii]);
        KOutMsg ("\n");
    }
#endif
    rc = KCipherEncryptCBC (self->ciphers.master, d->key, e->key,
                            sizeof (d->key) / sizeof (ivec));
    if (rc)
        return rc;
#if 0
    {
        unsigned iii;
        KOutMsg ("v2 encrypted key\n");
        for (iii = 0; iii < sizeof (e->key); +++ iii) 
            KOutMsg ("%2.2x ", e->key[iii]);
        KOutMsg ("\n");
    }
#endif
    /*
     * encrypt the data, offset and valid values
     */
    rc = KCipherEncryptCBC (self->ciphers.block, 
                            d->data, e->data,
                            (sizeof d->data + sizeof d->u) / sizeof (ivec));
    if (rc)
        return rc;

    d->u.valid = saved_valid;

    crc = CRC32 (0, e, (char*)(&e->crc)-(char*)e);

    self->block.crc = crc;

    if (self->bswap)
    {
        assert (sizeof crc == 4);
        crc = bswap_32 (crc);
    }
    e->crc_copy = e->crc = crc;

/*     KOutMsg ("%s: %lu %lu %lu ", __func__, self->foot.block_count, self->foot.crc_checksum,self->block.id); */
    if (self->foot.block_count <= self->block.id)
        self->foot.block_count = self->block.id + 1;

    if (!self->sought)
        self->foot.crc_checksum += crc;

/*     KOutMsg ("%lu %lu\n", __func__, self->foot.block_count, self->foot.crc_checksum); */

    return 0;
}


/* ----------
 * BlockDecrypt
 *   decrypt decrypts the data from a KEncFileBlock into the KEncFileBlock
 *   in the KEncFile object
 *
 * Not thread safe - use of cipher schedules ivec and block key in the ciphers
 *
 * If this function were a protected region where only one thread could be in
 * the body of this function at a time it could be made thread safe.
 */
static
rc_t KEncFileBlockDecrypt (KEncFile * self, KEncFileBlockId bid,
                           const KEncFileBlock * e, KEncFileBlock * d)
{
    KEncFileIVec ivec;
    rc_t rc;

    d->id = e->id;

    /* create the initialization vector for this block */
    KEncFileIVecInit (bid, &ivec);

    /*
     * set the ivec for both the master and data block ciphers
     */
    rc = KCipherSetDecryptIVec (self->ciphers.master, &ivec);
    if (rc)
        return rc;

    rc = KCipherSetDecryptIVec (self->ciphers.block, &ivec);
    if (rc)
        return rc;

    /* 
     * decrypt the block key and initial vector using the user key and 
     * the computer ivec
     */
    rc = KCipherDecryptCBC (self->ciphers.master, e->key, d->key,
                            (sizeof e->key) / sizeof ivec);
    if (rc)
        return rc;

    /* 
     * now create the AES key for the block from the newly decrypted 
     * block key
     */
    rc = KCipherSetDecryptKey (self->ciphers.block, d->key,
                               sizeof d->key);
    if (rc)
        return rc;

    rc = KCipherDecryptCBC (self->ciphers.block, e->data, d->data,
                            (sizeof e->data + sizeof e->u) / sizeof ivec);
    if (rc)
        return rc;

    if (self->bswap)
    {
        assert (sizeof d->u.valid == 2);
        d->u.valid = bswap_16 (d->u.valid);
    }

    if (d->u.valid >= sizeof d->data)
        d->u.valid = sizeof d->data;
    else
        memset (d->data + d->u.valid, 0, sizeof d->data - d->u.valid);

    return rc;
}


/*
 * if not decrypting block can be NULL
 */

/*
  TBD: figure out rational way to handle bad password making blocks look really weird
  If we have one that looks like a partial but isn't the last block -what do we do?
*/

static
rc_t KEncFileBlockRead (KEncFile * self, KEncFileBlock * block, 
                        KEncFileBlockId block_id, bool validate)
{
    union
    {
        KEncFileBlock b;
        KEncFileFooter f;
    } u;
    size_t num_read;
    uint64_t epos, dpos;
    rc_t vrc, rc = 0;
    bool missing;

    assert (self);
    assert ((validate == false) || (validate == true));
    /* we should be decrypting or validating - maybe both */
    assert ((block != NULL) || (validate == true));

    /* translate block id into both encrypted and decrypted addresses */
    epos = BlockId_to_CiphertextOffset ( block_id );
    dpos = BlockId_to_PlaintextOffset ( block_id );

    missing = false;

    /* clear out target block */
    if (block != NULL)
        memset (block, 0, sizeof * block);

    rc = KEncFileBufferRead (self, epos, &u.b, sizeof u.b, &num_read);
    if (rc)
    {
        PLOGERR (klogErr, (klogErr, rc, "Failure to read block '$(B)' at '$(E)'"
                           " in encrypted file decrypted at '$(D)",
                           "B=%lu,E=%lu,D=%lu", block_id, epos, dpos));
    }
    else
    {
        switch (num_read)
        {
        case 0:
            self->eof = true;
            /* ain't got no block here */
            break;

        default:
            /* Invalid size */
            rc = RC (rcKrypto, rcFile, rcReading, rcBuffer, rcInsufficient);
            PLOGERR (klogErr, (klogErr, rc, "Failure to read full block '$(B)' "
                               "at '$(E)' in encrypted file decrypted at '$(D)",
                               "B=%lu,E=%lu,D=%lu", block_id, epos, dpos));
            break;

        case sizeof (u.f):
            /* footer */
            if (validate) /* validate before checking as missing */
            {
                KEncFileFooterSwap (self, &u.f);

                if (u.f.block_count != block_id)
                {
                    vrc = RC (rcKrypto, rcFile, rcValidating, rcSize,
                              rcIncorrect);
                    PLOGERR (klogErr,  (klogErr, vrc, "read footer block count "
                                        "'$(B)' does not match actual block "
                                        "count '$(A)'", "B=%lu,A=%lu",
                                        u.f.block_count, block_id));
                    if ( rc == 0 )
                        rc = vrc;
                }
                vrc = KEncFileFooterValidate (self, u.f.block_count,
                                              u.f.crc_checksum);

#if 1
                if ( rc == 0 )
                    rc = vrc;
#else
                /* what is the significance of a NULL block? */
                if (block == NULL)
                    rc = vrc;
#endif
            }

            /* is it a "missing" footer? */
            /* or if the footer appears invalid - make it "valid" */
            if (((missing = BufferAllZero(&u.f, sizeof u.f)) == true) ||
                (self->foot.block_count != block_id))
            {
/*                 self->foot.block_count = block_id; */
                self->foot.crc_checksum = 0;
            }

            /* force some values though they might already be known */
            self->enc_size = epos + sizeof u.f;
            self->size_known = true;
            self->dec_size = dpos;
            self->eof = true;
            break;

        case sizeof (u.b):
            /* block */
            self->eof = false;
            /* is it a "missing" block? */
            if ((missing = BufferAllZero(&u.b, sizeof u.b)) == true)
            {
                if (validate)
                {
                    vrc = RC (rcKrypto, rcFile, rcValidating,
                                   rcData, rcNull);

                    PLOGERR (klogErr, (klogErr, vrc, "read missing block at "
                                       "block number '$(I)' encrypted position "
                                       "$(E) decrypted postion $(D)",
                                       "I=%lu,E=%lu,D=%lu", block_id, epos,
                                       dpos));
                    if ( rc == 0 )
                        rc = vrc;
                }
                u.b.id = block_id;
                u.b.u.valid = sizeof u.b.data;

                /* if we can only learn of the size by reading and are thus scanning
                 * through the current decrypt position must be the current known
                 * decrypted side size
                 */
                if (!self->size_known)
                {
                    assert (dpos == self->dec_size);
                    self->dec_size = dpos + sizeof u.b.data;
                }
                /*
                 * if we know the decrypted size and it is less than what we
                 * read, adjust the valid.  BUT this must not be for the block read
                 * for the last block to know the decrypted size. A chicken and egg
                 * problem.
                 */
                else if ((self->dec_size >= dpos) &&
                         (self->dec_size < dpos + sizeof u.b.data))
                    u.b.u.valid  = (uint16_t)(self->dec_size - dpos);
            }

            /* we read a full block that wasn't all zeroes */
            else
            {
                /* since we've chosen not to standardize the file format byte ordering */
                if (self->bswap)
                {
                    assert (sizeof u.b.crc == 4);
                    u.b.crc = bswap_32 (u.b.crc);
                    u.b.crc_copy = bswap_32 (u.b.crc_copy);

                    assert (sizeof u.b.id == 8);
                    u.b.id = bswap_64 (u.b.id);
                }

                if (validate)
                {
                    uint32_t crc;

                    if (block_id != u.b.id)
                    {
                        vrc = RC (rcKrypto, rcFile, rcValidating, rcIndex,
                                  rcIncorrect);
                        PLOGERR (klogErr, (klogErr, vrc, "error validating id "
                                           "for block '$(BID)' is not $(C2)", 
                                           "BID=%lu,C2=%lu", block_id, u.b.id));
                        if ( rc == 0 )
                            rc = vrc;
                    }

                    crc = CRC32 (0, &u.b, (char*)&u.b.crc - (char*)&u.b);

                    if (crc != u.b.crc)
                    {
                        vrc = RC (rcKrypto, rcFile, rcValidating, rcChecksum, rcCorrupt);
                        PLOGERR (klogErr,
                                 (klogErr,
                                  vrc,
                                  "error validating crc for block '$(BID)' $(C1) is not $(C2)", 
                                  "BID=%lu,C1=0x%X,C2=0x%X", block_id,
                                  crc, u.b.crc));
                        if ( rc == 0 )
                            rc = vrc;
                    }
                    if (crc != u.b.crc_copy)
                    {
                        vrc = RC (rcKrypto, rcFile, rcValidating, rcChecksum, rcCorrupt);
                        PLOGERR (klogErr,
                                 (klogErr,
                                  vrc,
                                  "error validating crc_copy for block '$(BID)' $(C1) is not $(C2)", 
                                  "BID=%lu,C1=0x%X,C2=0x%X", block_id,
                                  crc, u.b.crc_copy));
                        if ( rc == 0 )
                            rc = vrc;
                    }
                }
            }
            if (self->sought == false)
            {
                if (block_id == 0)
                {
                    self->foot.block_count = 1;
                    self->foot.crc_checksum = u.b.crc;
                }
                else
                {
                    ++self->foot.block_count;
                    self->foot.crc_checksum += u.b.crc;
                }
            }

            if (block != NULL)
            {
                if (missing)
                {
                    if (self->dad.write_enabled == false)
                        rc = RC (rcKrypto, rcFile, rcReading, rcData, rcIncomplete);
                    else
                    {
                        memmove (block, &u.b, sizeof u.b);
                        rc = 0;
                    }
                }
                else
                {
                    rc = KEncFileBlockDecrypt (self, block_id, &u.b, block);
                    if (rc == 0)
                    {
                        if (block_id == 0)
                        {
                            rc_t sra = KFileIsSRA ((const char *)block->data, block->u.valid);
                            self->sra =  (sra == 0);
                        }

                        if (!self->size_known)
                        {
                            assert (dpos == self->dec_size);
                            self->dec_size = dpos + sizeof u.b.u.valid;
                            if (u.b.u.valid != sizeof u.b.data)
                            {
                                self->size_known = true;
                                self->enc_size = epos + sizeof u.b + sizeof self->foot;
                            }
                        }
                    }
                }
            }
            break;
        }
    }
    return rc;
}


/*
 * Take a dirty block, encrypt it and write it to the backing file
 */
static
rc_t KEncFileBlockFlush (KEncFile * self, KEncFileBlock * dec_block)
{
    rc_t rc = 0;

    assert (self);
    assert (dec_block);


    if (dec_block->id == 0)
    {
        rc = KFileIsSRA ((const char *)(dec_block->data), sizeof (KSraHeader));

        /* we wait ALL the way until we try to flush the first block before we set
         * the sra flag for write only files.
         * we get it when we read the first block otherwise.
         */
        if (self->sra != (rc == 0))
        {
            self->sra = (rc == 0);
            self->has_header = false;
        }
    }
    if ((dec_block->id == 0) || (self->seekable))
    {
        if (!self->has_header)
        {
            if (!self->swarm)
            {
                rc = KEncFileHeaderWrite (self);
                if (rc)
                    return rc;
            }
            else if (dec_block->id == 0)
                self->enc_size = sizeof (KEncFileHeader);
        }
    }

/*     if (self->dirty) */
    {
        KEncFileBlock enc_block;

        rc = KEncFileBlockEncrypt (self, dec_block, &enc_block);
        if (rc == 0)
        {
            KEncFileBlockId block_id;
            uint64_t pos;
            size_t num_writ;

            block_id = dec_block->id;

            pos = BlockId_to_CiphertextOffset ( block_id );

            rc = KEncFileBufferWrite (self, pos, &enc_block, sizeof enc_block,
                                      &num_writ);

            if (rc)
                PLOGERR (klogErr, (klogErr, rc,
                                   "error writing encrypted block '$(B)'",
                                   "B=%lu", block_id));

            else if (num_writ != sizeof enc_block)
            {
                rc = RC (rcKrypto, rcFile, rcWriting, rcBuffer, rcInsufficient);
                PLOGERR (klogErr, (klogErr, rc, "error writing encrypted block "
                                   "'$(B)' wrote '$(Z)' not '$(Y)'",
                                   "B=%lu, Z=%zu", block_id, num_writ,
                                   sizeof enc_block));
            }
            else
                self->dirty = false;
        }
    }

    return rc;
}


/* ----------------------------------------------------------------------
 * Interface Functions
 *
 * Destroy
 *
 */
static
rc_t CC KEncFileDestroy (KEncFile *self)
{
    rc_t rc1 = 0;
    rc_t rc2 = 0;
    rc_t rc3 = 0;
    rc_t rc4 = 0;
    rc_t rc5 = 0;
    rc_t rc6 = 0;

    assert (self);

    if (self->dad.write_enabled)
    {
        /*
         * write the header if we've written nothing to an empty file
         * or if we've written something which will mean a change to v2
         * of the encrypted file format
         */
        if ((self->dec_size == 0) || (self->seekable && self->changed) ||
            ((self->dec_size == 0) && (!self->dad.read_enabled) && self->changed && (self->has_header == false)))
/* SMURF IX
            (self->has_header == false))
 */
            rc1 = KEncFileHeaderWrite (self);

        /* write any dirty block */
        if (self->dirty)
            rc2 = KEncFileBlockFlush (self, &self->block);

        /* [re]write footer */
        if (self->changed)
            rc3 = KEncFileFooterWrite (self);
    }
    rc4 = KFileRelease (self->encrypted);
    rc5 = KCipherRelease (self->ciphers.master);
    rc6 = KCipherRelease (self->ciphers.block);

    free (self);
    
    if (rc1)
        return rc1;
    if (rc2)
        return rc2;
    if (rc3)
        return rc3;
    if (rc4)
        return rc4;
    if (rc5)
        return rc5;
    return rc6;
}


/* ----------------------------------------------------------------------
 * GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 *
 * We do not allow this for read, write or update as you can not memory map the 
 * unencrypted file in a meaningful way.
 */
static
struct KSysFile *CC KEncFileGetSysFile (const KEncFile *self, uint64_t *offset)
{
    assert (self);
    assert (offset);

    return NULL;
}


/* ----------------------------------------------------------------------
 * RandomAccess
 *
 *  returns 0 if random access, error code otherwise
 */
static
rc_t CC KEncFileRandomAccess (const KEncFile *self)
{
    assert (self != NULL);
    assert ((self->seekable == true) || (self->seekable == false));

    /* we checked for random access in the contructor */
    if (self->seekable)
        return 0;

    return RC (rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t CC KEncFileSize (const KEncFile *self, uint64_t *size)
{
    if (!self->size_known)
        return RC (rcKrypto, rcFile, rcAccessing, rcSize, rcUnsupported);

    *size = self->dec_size;
    return 0;
}


/* ----------------------------------------------------------------------
 * SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 *
 * This is the size of the decrypted payload not of the encrypted file
 */
static
rc_t KEncFileSetSizeBlockFull (KEncFile *self, uint64_t block_id)
{
    if ((self->block.id == block_id) && (self->block.u.valid != 0))
    {
        if (self->block.u.valid < sizeof self->block.data)
        {
            self->block.u.valid = sizeof self->block.data;
            self->dirty = true;
        }
        else
            assert (self->block.u.valid  == sizeof self->block.data);
    }
    else
    {
        KEncFileBlock block;
        rc_t rc;

        rc = KEncFileBlockRead (self, &block, block_id, false);
        if (rc)
            return rc;

/*         if (self->block.u.valid  != sizeof self->block.data) */
/*         { */
/*             OUTMSG (("%s: %u %u\n", __func__,self->block.u.valid,sizeof self->block.data)); */
/*             OUTMSG (("%s: %lu %lu\n", __func__,self->block.id,block_id)); */
/*         } */

        /* only change block if not "missing" */
        if (BufferAllZero (&block, sizeof block) == false)
        {
            assert (block.id == block_id);

            /* only change if not already full - shouldnt get here if not? */
            if (block.u.valid < sizeof block.data)
            {
                self->changed = self->sought = true;
                block.u.valid = sizeof block.data;

                rc = KEncFileBlockFlush (self, &block);
                if (rc)
                    return rc;
            }
            else
                assert (self->block.u.valid  == sizeof self->block.data);
        }
    }
    return 0;
}


static
rc_t KEncFileSetSizeBlockPartial (KEncFile *self, uint64_t block_id, uint32_t valid)
{
    if ((self->block.id == block_id) && (self->block.u.valid != 0))
    {
        if (self->block.u.valid != valid)
        {
            self->block.u.valid = valid;
            self->dirty = true;
        }
    }
    else
    {
        KEncFileBlock block;
        rc_t rc;

        rc = KEncFileBlockRead (self, &block, block_id, false);
        if (rc)
            return rc;

        /* only change block if not "missing" */
        if (BufferAllZero (&block, sizeof block) == false)
        {
            assert (block.id == block_id);

            if (block.u.valid != valid)
            {
                self->changed = self->sought = true;
                block.u.valid = valid;
                
                rc = KEncFileBlockFlush (self, &block);
                if (rc)
                    return rc;
            }
        }
    }
    return 0;
}


static
rc_t KEncFileSetSizeInt (KEncFile *self, uint64_t dec_size)
{
    uint64_t trim_size = 0;
    uint64_t enc_size;
    bool do_size = true;

    rc_t rc = 0;

    /* should we not have been called? */
    if ((dec_size == self->dec_size) && (self->enc_size != 0))
        return 0;

    /* if wiping out the whole file */
    if (dec_size == 0)
    {
        trim_size = sizeof (KEncFileHeader);
        enc_size = (sizeof (KEncFileHeader) + sizeof (KEncFileFooter));

        /* if we did clear out the RAM structures to match */
        memset (&self->block, 0, sizeof self->block);
        memset (&self->foot, 0, sizeof self->foot);
        self->dirty = false;
        self->size_known = true;
        self->bswap = false;
        self->changed = true;
        self->sought = false;
        self->has_header = false;
        self->version = eCurrentVersion;
    }
    else
    {
        KEncFileBlockId new_bid;    /* block id of new last block */
        KEncFileBlockId new_fid;    /* block id of new footer / block count */
        uint32_t        new_doff;   /* bytes into last partial block */

        /*
         * determine sizes of decrypted virtual file
         * and encrypted 'real' file
         */

        /*
          NB - the following code utilizes a function for converting
          an OFFSET to a zero-based block-id. However, it passes in a size.
          By examining new_doff, it detects the case where "dec_size" is an
          exact multiple of plaintext block size, and takes the return to
          be a "block-id" of the footer, and new_bid to be the last block.

          in the case where "new_doff" is zero, "new_bid" will be the effective
          id of the footer, and "new_bid" will need to be adjusted to the previous
          full block.

          in the case where "new_doff" is not zero, "new_bid" will be the
          last data block id, and the footer will be one beyond.

          although the code utilizes incorrect and misleading primitives,
          it works.
         */

        new_fid = new_bid = PlaintextOffset_to_BlockId (dec_size, &new_doff);
        if (new_doff == 0)
            --new_bid; /* exactly fills a block */
        else
            ++new_fid;  /* leaves a partial block */
    
        enc_size = BlockId_to_CiphertextOffset ( new_fid ) + sizeof self->foot;

        /* are we starting with an empty file? It's easy if we are */
        if (self->dec_size == 0)
        {
            /* TBD - this looks incorrect... what about KEncFileHeader?
               the code below would use BlockId_to_CiphertextOffset()
             */
            trim_size = sizeof (KEncFileHeader);

            /* if we did clear out the RAM structures to match */
            memset (&self->block, 0, sizeof self->block);
            self->bswap = false;
            self->changed = true;
            self->version = eCurrentVersion;
        }
        else
        {
            KEncFileBlockId old_bid;    /* block id of old last block */
            KEncFileBlockId old_fid;    /* block id of old footer / block count */
            uint32_t        old_doff;   /* bytes into last partial block */

            old_fid = old_bid = PlaintextOffset_to_BlockId (self->dec_size, &old_doff);
            if (old_doff == 0)
                --old_bid; /* exactly fills a block */
            else
                ++old_fid;  /* leaves a partial block */

            /* are we only changing the last block? */
            if (old_bid == new_bid)
            {
                assert ((self->dad.read_enabled == false) || (self->enc_size == enc_size));

                if (new_doff == 0)
                {
                    /* change from partial to full last block */
                    rc = KEncFileSetSizeBlockFull (self, new_bid);
                }
                else
                {
                    /* resize last block */
                    rc = KEncFileSetSizeBlockPartial (self, new_bid, new_doff);
                }

                /* no need to resize underlying file */
                do_size = false;
            }
            else
            {

                /* truncating the file? */
                if (dec_size < self->dec_size)
                {
                    trim_size = BlockId_to_CiphertextOffset ( new_fid );

                    /* do we throw away the block in the object? */
                    if (self->block.id > new_bid)
                    {
                        self->dirty = false;
                        memset (&self->block, 0, sizeof self->block);
                    }

                    /* we only change the new last block if its now partial */
                    if ( new_doff != 0 )
                        rc = KEncFileSetSizeBlockPartial (self, new_bid, new_doff);

                }
                /* expanding the file */
                else
                {
                    assert (dec_size > self->dec_size);

                    trim_size = BlockId_to_CiphertextOffset ( old_fid );

                    /* make old last block a full block if it wasn't already */
                    if ( old_doff != 0 )
                        rc = KEncFileSetSizeBlockFull (self, old_bid);
                }
            }
        }
    }
    if (rc == 0)
    {
        if (do_size)
        {
            /* first trim for some reason... sparse files? */
            rc = KFileSetSize (self->encrypted, trim_size);
            if (rc)
                LOGERR (klogErr, rc, "failure to trim size of encrypted file");
            else
            {
                /* now extend to encrypted size */
                rc = KFileSetSize (self->encrypted, enc_size);
                if (rc)
                    LOGERR (klogErr, rc, "failure to file size of encrypted file");
            }
        }
        if (rc == 0)
        {
            self->enc_size = enc_size;
            self->dec_size = dec_size;
        }
    }
    return rc;
}


static
rc_t CC KEncFileSetSize (KEncFile *self, uint64_t dec_size)
{
    assert (self);
    assert (self->encrypted);

    if (self->dad.write_enabled == false)
        return RC (rcKrypto, rcFile, rcResizing, rcFile, rcNoPerm);

    if ((self->seekable == false) || (self->size_known == false))
        return RC(rcKrypto, rcFile, rcAccessing, rcFunction, rcUnsupported);

    /* silently ignore changes in size that don't change the size */
    if (dec_size == self->dec_size)
        return 0;

    else
        return KEncFileSetSizeInt (self, dec_size);
}


/* ----------------------------------------------------------------------
 * Read
 *  read file from known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually read
 */
static
rc_t CC KEncFileRead (const KEncFile *cself,
                      uint64_t pos,
                      void *buffer,
                      size_t bsize,
                      size_t *num_read)
{
    KEncFile * self = (KEncFile *)cself; /* mutable values */
    uint64_t max_bid;
    uint64_t block_id;
    uint32_t offset;
    rc_t rc = 0;

    assert (self); /* checked in file.c KFileRead() */
    /* pos can be 'anything' */
    assert (buffer); /* checked in file.c KFileRead() */
    assert (bsize); /* checked in file.c KFileRead() */
    assert (num_read); /* checked in file.c KFileRead() */

    assert (self->dad.read_enabled);
    assert (self->encrypted);

    /* do we have a decrypted_size? */

    block_id = PlaintextOffset_to_BlockId (pos, &offset);

    switch ( ( uint32_t ) self->size_known)
    {
    case 0:

        max_bid = EncryptedPos_to_BlockId (self->enc_size, NULL, NULL);

        /* if past end of file as we know it so far quick out */
        if (max_bid <= block_id)
            return 0;
        break;

    case 1:
        /* if past end of file quick out */
        if (pos > self->dec_size)
            return 0;
        break;

    default:
        assert (0 && self->size_known);
        break;
    }

    /*
     * are we on the wrong block?
     * Or are do we need to read the first block?
     */
    if ((block_id != self->block.id) || (self->block.u.valid == 0))
    {
        if ((!self->seekable) && (self->block.id + 1 != block_id))
        {
            rc = RC (rcFS, rcFile, rcReading, rcOffset, rcIncorrect);
            PLOGERR (klogErr, (klogErr, rc, "attempt to seek in encryption write at"
                               " '$(O)' seek to '$(P)'", "O=%lu,P=%lu",
                               BlockId_to_CiphertextOffset(self->block.id), pos));
        }
        else
        {
            /* flush any dirty block */
            if (self->dirty)
            {
                assert (self->dad.write_enabled);
                rc = KEncFileBlockFlush (self, &self->block);
            }

            /* now try to read in a new block */
            if (rc == 0)
                rc = KEncFileBlockRead (self, &self->block, block_id, false);

            if (rc == 0)
            {
                uint64_t read_max;

                read_max = BlockId_to_PlaintextOffset ( block_id ) + self -> block . u . valid;
                if (self->dec_size < read_max)
                    self->dec_size = read_max;
            }
        }
    }

    /*
     * if we are trying to read past the end of the file 
     * return 0 with nothing read
     */
    if ((rc == 0) &&
        (block_id == self->block.id) &&
        (offset < self->block.u.valid))
    {
        size_t to_copy;

        to_copy = self->block.u.valid - offset;
        if (to_copy > bsize)
            to_copy = bsize;

        memmove (buffer, self->block.data + offset, to_copy);
        *num_read = to_copy;
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * Write
 *  write file at known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually written
 */
static
rc_t CC KEncFileWrite (KEncFile *self, uint64_t pos,
                       const void *buffer, size_t bsize,
                       size_t *pnum_writ)
{
    rc_t rc = 0;

    assert (self);
    assert (buffer);
    assert (bsize > 0);
    assert (pnum_writ);

    assert (self->dad.write_enabled);
    assert (self->encrypted);

/*     KOutMsg ("+++++\n%s: pos %lu\n",__func__,pos); */

    if (self->dec_size != pos)
    {
        /* write only does not allow seeks */
        if ((!self->dad.read_enabled) && (!self->swarm))
        {
            rc = RC (rcFS, rcFile, rcWriting, rcOffset, rcIncorrect);
            PLOGERR (klogErr, (klogErr, rc, "attempt to seek in encryption write at"
                               " '$(O)' seek to '$(P)'", "O=%lu,P=%lu",
                               BlockId_to_CiphertextOffset(self->block.id), pos));
        }
        else
            self->sought = true;
    }
    if (rc == 0)
    {
        uint64_t block_id;
        uint32_t offset;
        uint64_t block_max;
        uint64_t new_size;

        rc = 0;
        /* Block Id for this write */
        block_id = PlaintextOffset_to_BlockId (pos, &offset);

        block_max = BlockId_to_PlaintextOffset ( block_id + 1 );

        new_size = pos + bsize;
        if (new_size > block_max)
            bsize = block_max - new_size;


        /* is the new position beyond the current file length? */
        if ((new_size > self->dec_size) && (self->dad.read_enabled) && (!self->swarm))
        {
            rc = KEncFileSetSizeInt (self, new_size);
            if (rc)
                return rc;
        }

        /* are we going to a new block? */
        if ((block_id != self->block.id) || (self->block.u.valid == 0))
        {
            /* do we have sometihng to flush first? */
            if (self->dirty)
            {
                assert (self->dad.write_enabled);
                rc = KEncFileBlockFlush (self, &self->block);
            }

            if (rc == 0)
            {
                /* if we are going to over write the whole block */
                if ((!self->dad.read_enabled) ||
                    ((offset == 0) && (bsize >= sizeof (self->block.data))))
                {
                    memset (&self->block, 0, sizeof self->block);
                    self->block.id = block_id;
                }
                /* else try to fetch an existing block */
                else
                    rc = KEncFileBlockRead (self, &self->block, block_id, false);
            }
        }

        if (rc == 0)
        {
            /* we are at the right block and ready to write */
            uint32_t new_valid;
            size_t to_copy;

            /*
             * force block id to be right even if fetch was beyond end of 
             * existing file
             */
            self->block.id = block_id;

/*             to_copy = sizeof self->block.data - self->block.u.valid; */
            to_copy = sizeof self->block.data - offset;

            if (to_copy > bsize)
                to_copy = bsize;

            memmove (self->block.data + offset, buffer, to_copy);
            self->dirty = true;
            *pnum_writ = to_copy;

            new_valid = (uint32_t) ( offset + to_copy );
            if (new_valid > self->block.u.valid)
            {
                uint64_t new_size;

                self->block.u.valid = new_valid;

                new_size = pos + to_copy;
                if (new_size > self->dec_size)
                    self->dec_size = new_size;
            }

            if (self->swarm)
                rc = KEncFileBlockFlush (self, &self->block);


        }
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 *
 * Just return what the backing file says...
 */
static
uint32_t CC KEncFileType (const KEncFile *self)
{
    assert (self != NULL);
    assert (self->encrypted != NULL);

    return KFileType (self->encrypted);
}


/* ----------------------------------------------------------------------
 * KEncFileMake
 *  create a new file object
 */

/* ----------
 * KeysInit
 */
static
rc_t KEncFileCiphersInit (KEncFile * self, const KKey * key, bool read, bool write)
{
    KCipherManager * mgr;
    size_t z;
    rc_t rc;

    switch ( key->type)
    {
    default:
        return RC (rcKrypto, rcEncryptionKey, rcConstructing, rcParam, rcInvalid);

    case kkeyNone:
        return RC (rcKrypto, rcEncryptionKey, rcConstructing, rcParam, rcIncorrect);

    case kkeyAES128:
        z = 128/8; break;

    case kkeyAES192:
        z = 192/8; break;

    case kkeyAES256:
        z = 256/8; break;
    }
    rc = KCipherManagerMake (&mgr);
    if (rc == 0)
    {
        rc = KCipherManagerMakeCipher (mgr, &self->ciphers.master, kcipher_AES);
        if (rc == 0)
        {
            rc = KCipherManagerMakeCipher (mgr, &self->ciphers.block, kcipher_AES);
            if (rc == 0)
            {
                rc = KCipherSetDecryptKey (self->ciphers.master, key->text, z);
                if (rc == 0)
                {
                    rc = KCipherSetEncryptKey (self->ciphers.master, key->text, z);
                    if (rc == 0)
                        goto keep_ciphers;
                }
                KCipherRelease (self->ciphers.block);
                self->ciphers.block = NULL;
            }
            KCipherRelease (self->ciphers.master);
            self->ciphers.master = NULL;
        }
    keep_ciphers:
        KCipherManagerRelease (mgr);
    }
    return rc;
}



static const KFile_vt_v1 vtKEncFile =
{
    /* version */
    1, 1,

    /* 1.0 */
    KEncFileDestroy,
    KEncFileGetSysFile,
    KEncFileRandomAccess,
    KEncFileSize,
    KEncFileSetSize,
    KEncFileRead,
    KEncFileWrite,
    
    /* 1.1 */
    KEncFileType
};


static
rc_t  KEncFileMakeIntValidSize (uint64_t enc_size, bool w)
{

    if (enc_size == 0)
    {
        if (w)
            return 0;
    }
    else
    {
        uint64_t min_size = sizeof (KEncFileHeader) + sizeof (KEncFileFooter);
        uint64_t block_count;

        if (enc_size >= min_size)
        {
            block_count = EncryptedPos_to_BlockId (enc_size, NULL, NULL);

            if (enc_size - BlockId_to_CiphertextOffset (block_count) == sizeof (KEncFileFooter))
                return 0;
        }
    }
    return RC (rcKrypto, rcFile, rcConstructing, rcSize, rcIncorrect);
}


/* ----------
 * MakeInt
 *    common make for all encryptor/decryptors
 */
static
rc_t KEncFileMakeInt (KEncFile ** pself, KFile * encrypted,
                      bool r, bool w, bool v, bool s)
{
    uint64_t enc_size;
    rc_t rc = 0, orc;
    bool seekable;
    bool size_known;

    assert (pself);
    assert (encrypted);
    assert (((r == true) || (r == false)) &&
            ((w == true) || (w == false)) &&
            ((v == true) || (v == false)));

    /* must be able to do at elast one of read and write */
    assert (r || w);

    /* expecting to validate read only right now */
/*     assert ((v && r && !w) || (!v)); */

    if (w && ! encrypted->write_enabled)
    {
        rc = RC (rcKrypto, rcFile, rcConstructing, rcFile, rcReadonly);
        LOGERR (klogErr, rc, "Can not make a encryptor for a unwritable file");
    }
    if (r && ! encrypted->read_enabled)
    {
        orc = RC (rcKrypto, rcFile, rcConstructing, rcFile, rcWriteonly);
        LOGERR (klogErr, orc, "Can not make a decryptor for an unreadable file");
        if (rc == 0)
            rc = orc;
    }
    if (rc)
        return rc;

    /* determine whether the original file can tell us the size */
    rc = KFileSize (encrypted, &enc_size);
    if (rc == 0)
        size_known = true;

    else if (GetRCState(rc) == rcUnsupported)
        size_known = false;
    
    else
        return rc;

    if (!v && size_known)
    {
        rc = KEncFileMakeIntValidSize (enc_size, w);
        if (rc)
            return rc;
    }

    /* determine whether the original file allows seeks */
    rc = KFileRandomAccess (encrypted);
    if (rc == 0)
        seekable = true;

    else if (GetRCState(rc) == rcUnsupported)
        seekable = false;

    else
    {
        LOGERR (klogErr, rc, "error checking random access building "
                "encrypted file");
        return rc;
    }

    /* We are currently only supporting update on seekable and size_known original files */
    if (r && w && ((!seekable) || (!size_known)))
    {
        rc = RC (rcKrypto, rcFile, rcConstructing, rcFile, rcIncorrect);
        LOGERR (klogErr, rc, "encryptor/decryptor requires seek and size ability");
        return rc;
    }

    rc = KFileAddRef (encrypted);
    if (rc)
    {
        LOGERR (klogErr, rc, "Could not add reference to encrypted file");
        return rc;
    }
    else
    {
        KEncFile * self;

        /* allocate and zero out an object since we want much of it to be zeroed */
        self = calloc (1, sizeof *self);
        if (self == NULL)
        {
            rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
            LOGERR (klogSys, rc,
                    "out of memory creating encryptor and/or decryptor");
        }
        else
        {
            /* all KFiles get this initialization */
            rc = KFileInit (&self->dad, (const KFile_vt*)&vtKEncFile, "KEncFile", "no-name", r, w);
            if (rc)
                LOGERR (klogInt, rc, "error with init for encrypted file");

            else
            {
                self->encrypted = encrypted;
                self->swarm = s;

                /* write only or empty updatable */
                if ((!r) || (w && size_known && (enc_size == 0)))
                {
                    /* dummy size to make the SetSizeInt work */
                    self->enc_size = enc_size;
                    rc = KEncFileSetSizeInt (self, 0);
                    self->seekable = r && seekable;
                }
                else
                {
                    self->enc_size = enc_size;
                    self->seekable = seekable;
                    self->size_known = size_known;
                    rc = KEncFileHeaderRead (self);
                }
                if (rc == 0)
                {
                    *pself = self;
                    return 0;
                }
            }
            free (self);
        }
        KFileRelease (encrypted);
    }
    return rc;
}


static
rc_t KEncFileMakeSize (KEncFile *self)
{
    KEncFileBlockId fid;
    rc_t rc;

    assert (self->seekable);

    /*
     * turn the encrypted size into a block/offset
     * the offset should be 0 for a missing footer
     * or the size of a footer
     */
    fid = EncryptedPos_to_BlockId (self->enc_size, NULL, NULL);

    assert (BlockId_to_CiphertextOffset(fid) + sizeof (self->foot) == self->enc_size);

    if (fid == 0)
        self->dec_size = 0;

    else
    {
        KEncFileBlockId bid = fid - 1;
        KEncFileBlock b;

        /*
         * not calling this a seek as its not reading a data block
         * out of order that will be modified
         */
        rc = KEncFileBlockRead (self, &b, bid, false);
        if (rc)
            return rc;
        else
        {
            if (BufferAllZero(&b, sizeof b) == true)
                self->dec_size = BlockId_to_PlaintextOffset (bid) + 
                    sizeof self->block.data;

            else
                self->dec_size = BlockId_to_PlaintextOffset (bid) + b.u.valid;
        }
    }
    self->size_known = true;
    return 0;
}


/* ----------
 * MakeCmn
 * common parameter validation for all encryptor/decryptors
 */
static
rc_t KEncFileMakeCmn (KEncFile ** pself, KFile * encrypted, const KKey * key,
                      bool r, bool w, bool s)
{
    rc_t rc = 0, orc;

    assert (((r == true)||(r == false))&&((w == true)||(w == false)));
    assert (w || r);

    if (pself == NULL)
    {
        rc = RC (rcKrypto, rcFile, rcConstructing, rcSelf, rcNull);
        LOGERR (klogErr, rc,
                "pointer to self NULL when creating "
                "an encryptor/decryptor");
    }
    else
        *pself = NULL;

    if (encrypted == NULL)
    {
        orc = RC (rcFS, rcFile, rcConstructing, rcParam, rcNull);
        LOGERR (klogErr, orc, 
                "encrypted file not readable when creating "
                "an encryptor/decryptor");
        if (rc == 0)
            rc = orc;
    }

    if (key == NULL)
    {
        orc = RC (rcFS, rcFile, rcConstructing, rcParam, rcNull);
        LOGERR (klogErr, orc, 
                "key not supplied when creating an encryptor/decryptor");
        if (rc == 0)
            rc = orc;
    }

    CRC32Init();    /* likely to be called way too often */

    switch (key->type)
    {
    default:
        orc = RC (rcFS, rcFile, rcConstructing, rcParam, rcInvalid);
        PLOGERR (klogErr,
                 (klogErr, orc, "invalid key type '$(T)' should be "
                  "kkeyAES128(1), kkeyAES192(2) or kkeyAES256(3)",
                  "T=%u", key->type));
        if (rc == 0)
            rc = orc;
        break;

    case kkeyAES128:
    case kkeyAES192:
    case kkeyAES256:
        break;
    }
    if (rc == 0)
    {
        KEncFile * self;

        assert ((r == true) || (r == false));
        assert ((w == true) || (w == false));

        rc = KEncFileMakeInt (&self, encrypted, r, w, false, s);
        if (rc == 0)
        {
            rc = KEncFileCiphersInit (self, key, r, w);
            if (rc == 0)
            {
                
                if (self->seekable && self->size_known)
                    rc = KEncFileMakeSize (self);

                if (rc == 0)
                {
                    *pself = self;
                    return 0;
                }
            }
            KFileRelease (&self->dad);
        }
    }
    return rc;
}


/* ----------
 * Read mode is fully seekable if the underlying KFile is seekable some
 * integrity checking will not be performed in allowing this seeking.
 */
LIB_EXPORT rc_t CC KEncFileMakeRead_v2 (const KFile ** pself, 
                                        const KFile * encrypted,
                                        const KKey * key)
{
    KEncFile * self;
    rc_t rc;

    /*
     * casting encrypted dowsn't actually make it writable
     * it just lets us use a common constructor
     */
    rc = KEncFileMakeCmn (&self, (KFile *)encrypted, key, true, false, false);
    if (rc)
        LOGERR (klogErr, rc, "error constructing decryptor");

    else
        *pself = &self->dad;

    return rc;
}


/* ----------
 * Write mode encrypted file can only be written straight through form the
 * first byte to the last.
 *
 * Existing content is lost.
 */
LIB_EXPORT rc_t CC KEncFileMakeWrite_v2 (KFile ** pself, 
                                         KFile * encrypted,
                                         const KKey * key)
{
    KEncFile * self;
    rc_t rc;

    rc = KEncFileMakeCmn (&self, encrypted, key, false, true, false);
    if (rc)
        LOGERR (klogErr, rc, "error constructing encryptor");

    else
        *pself = &self->dad;

    return rc;
}


LIB_EXPORT rc_t CC KEncFileMakeUpdate_v2 (KFile ** pself, 
                                          KFile * encrypted,
                                          const KKey * key)
{
    KEncFile * self;
    rc_t rc;

/*     static int count = 0; */

/*     KOutMsg ("%s: %d\n",__func__,++count); */

    rc = KEncFileMakeCmn (&self, (KFile *)encrypted, key, true, true, false);
    if (rc)
        LOGERR (klogErr, rc, "error constructing encryptor/decryptor");

    else
        *pself = &self->dad;

    return rc;
}


/* ----------
 * Swarm mode encrypted file can be writtenout of order but the footer is not
 * handled automatically
 */
LIB_EXPORT rc_t CC KEncFileMakeBlock_v2 (KFile ** pself, 
                                         KFile * encrypted,
                                         const KKey * key)
{
    KEncFile * self;
    rc_t rc;

/*     static int count = 0; */

/*     KOutMsg ("%s: %d\n",__func__,++count); */

    rc = KEncFileMakeCmn (&self, (KFile *)encrypted, key, false, true, true);
    if (rc)
        LOGERR (klogErr, rc, "error constructing encryptor/decryptor");

    else
        *pself = &self->dad;

    return rc;
}


/* ----------
 * Validate mode is useful only for the KFileEncValidate function
 */
static
rc_t KEncFileMakeValidate (KEncFile ** pself, const KFile * encrypted)
{
    KEncFile * self;
    rc_t rc;

    assert (pself);
    assert (encrypted);

    rc = KEncFileMakeInt (&self, (KFile*)encrypted, true, false, true, false);
    if (rc)
        LOGERR (klogErr, rc, "error making KEncFile");
    else
    {
        rc = KEncFileHeaderRead (self);
        if (rc)
            LOGERR (klogErr, rc, "error reading encrypted file header");
        else
        {
            *pself = self;
            return 0;
        }
    }
    *pself = NULL;
    return rc;
}


/* ======================================================================
 * Interface extensions
 */


/* ----------
 * Validate mode can not be read or written.
 * Upon open the whole file is read from begining to end and all CRC
 * and other integrity checks are performed immedaitely
 *
 * This will fail if the file being tested is not "at position 0" and can not
 * be sought back to 0.
 */



LIB_EXPORT rc_t CC KEncFileValidate_v2 (const KFile * encrypted)
{
    KEncFile * file;
    rc_t rc = 0;

    /* fail if a NULL parameter: can't validate all addresses */
    if (encrypted == NULL)
    {
        rc = RC (rcKrypto, rcFile, rcValidating, rcParam, rcNull);
        LOGERR (klogErr, rc, "encrypted file was null when trying to validate");
        return rc;
    }

    /* file header is validated within the call to Make Validate */
    rc = KEncFileMakeValidate (&file, encrypted);
    if (rc)
        LOGERR (klogErr, rc,
                "unable to validate encrypted file due to "
                "inability to open as encrypted file");
    else
    {
        uint64_t pos;             /* position within the encrypted file */
        uint64_t block_count = 0; /* how many blocks have we read */

        /* loop through all data blocks */
        pos = sizeof (KEncFileHeader);
        for (block_count = 0; ; ++block_count)
        {
            rc_t vrc;
            STSMSG (2, ("reading block '%u' at '%lu'", block_count,
                        BlockId_to_CiphertextOffset(block_count)));
           
            vrc = KEncFileBlockRead (file, NULL, block_count, true);
            if (vrc != 0)
            {
                if ( rc == 0 )
                    rc = vrc;
                if ( GetRCContext( vrc ) != rcValidating )
                {
                    STSMSG (2, ("read error at block '%u'", block_count));
                    break;
                }
            }
            if (file->eof)
            {
                STSMSG (2, ("block '%u' was end", block_count));
                break;
            }
            pos += sizeof (KEncFileData);
        }   
        KFileRelease (&file->dad);
    }
    return (rc);
}


LIB_EXPORT rc_t CC KEncFileHeaderWrite_v2 (KFile * dad)
{
    rc_t rc;

    if (dad->vt != (const KFile_vt*)&vtKEncFile)
    {
        rc = RC (rcKrypto, rcFile, rcWriting, rcType, rcIncorrect);
        LOGERR (klogErr, rc, "file not an encryptor requested writing header");
    }
    else
    {
        KEncFile * self;
        self = (KEncFile*)dad;

        rc = KEncFileHeaderWrite (self);
    }
    return rc;
}

LIB_EXPORT rc_t CC KEncFileFooterWrite_v2 (KFile * dad)
{
    rc_t rc;

    if (dad->vt != (const KFile_vt*)&vtKEncFile)
    {
        rc = RC (rcKrypto, rcFile, rcWriting, rcType, rcIncorrect);
        LOGERR (klogErr, rc, "file not an encryptor requested writing footer");
    }
    else
    {
        KEncFile * self;
        self = (KEncFile*)dad;

        rc = KEncFileFooterWrite (self);
    }
    return rc;
}

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
 *      the file is definitely not a KEncFile type encrypted file.
 *     
 * RC (rcFS, rcFile, rcIdentifying, rcParam, rcNull)
 *      bad parameters in the call
 *
 * RC (rcFS, rcFile, rcIdentifying, rcBuffer, rcInsufficient)
 *      not a large enough buffer to make an identification
 */
LIB_EXPORT rc_t CC KFileIsEnc_v2 (const char * buffer, size_t buffer_size)
{
    KEncFileHeader header;
    size_t count;
    bool byte_swapped;

    if ((buffer == NULL) || (buffer_size == 0))
        return RC  (rcFS, rcFile, rcIdentifying, rcParam, rcNull); 

    /* must have the signature to consider it an Encrypted file */
    if (buffer_size < sizeof (header.file_sig))
        return RC (rcFS, rcFile, rcIdentifying, rcBuffer, rcInsufficient); 

    if ((memcmp (buffer, &const_header.file_sig, sizeof const_header.file_sig ) != 0) &&
        (memcmp (buffer, &const_header_sra.file_sig, sizeof const_header_sra.file_sig ) != 0))
        return SILENT_RC (rcFS, rcFile, rcIdentifying, rcFile, rcWrongType); 

    /* can we also check the byte order? It's okay if we can't */
    if (buffer_size < sizeof header.file_sig + sizeof header.byte_order)
        return 0; 
    
    count = buffer_size > sizeof header ? sizeof header : buffer_size;

    memmove (&header, buffer, count);

    if (header.byte_order == const_header.byte_order)
        byte_swapped = false;

    else if (header.byte_order == const_bswap_header.byte_order)
        byte_swapped = true;

    /* but if it's not we fail with a different error */
    else
        return RC (rcFS, rcFile, rcIdentifying, rcFile, rcOutoforder); 

    /* can we check the version as well? It's okay if we can't */
    if (buffer_size < sizeof (header))
        return 0; 

    assert (sizeof (header.version) == 4);
    if (byte_swapped)
        header.version = bswap_32(header.version);

    /* and it's a different error if the version is not within our range */
    if ((header.version <= 0) || (header.version > eCurrentVersion))
        return RC (rcKrypto, rcFile, rcClassifying, rcFile, rcBadVersion);

    return 0;
}


LIB_EXPORT rc_t CC KFileIsSraEnc (const char * buffer, size_t buffer_size)
{
    KEncFileHeader header;
    size_t count;
    bool byte_swapped;

    if ((buffer == NULL) || (buffer_size == 0))
        return RC  (rcFS, rcFile, rcIdentifying, rcParam, rcNull); 


    if (buffer_size < sizeof (header.file_sig))
        return RC (rcFS, rcFile, rcIdentifying, rcBuffer, rcInsufficient); 

    if (memcmp (buffer, &const_header_sra.file_sig, sizeof const_header.file_sig ) != 0)
        return RC (rcFS, rcFile, rcIdentifying, rcFile, rcWrongType); 

    if (buffer_size < sizeof header.file_sig + sizeof header.byte_order)
        return 0; 
    
    count = buffer_size > sizeof header ? sizeof header : buffer_size;

    memmove (&header, buffer, count);

    if (header.byte_order == const_header.byte_order)
        byte_swapped = false;

    else if (header.byte_order == const_bswap_header.byte_order)
        byte_swapped = true;

    else
        return RC (rcFS, rcFile, rcIdentifying, rcFile, rcOutoforder); 

    if (buffer_size < sizeof (header))
        return 0; 

    assert (sizeof (header.version) == 4);
    if (byte_swapped)
        header.version = bswap_32(header.version);

    if ((header.version <= 0) || (header.version > eCurrentVersion))
        return RC (rcKrypto, rcFile, rcClassifying, rcFile, rcBadVersion);

    return 0;
}

/* end of file encfile.c */



