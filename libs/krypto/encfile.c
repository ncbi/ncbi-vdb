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
#include <krypto/key.h>
#include <krypto/ciphermgr.h>
#include <krypto/cipher.h>
#include "aes-priv.h"
#include "encfile-priv.h"

#include <klib/rc.h>
#include <klib/checksum.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/debug.h>
#include <klib/vector.h>
#include <klib/status.h>
#include <kfs/file.h>

#include <sysalloc.h>

#include <byteswap.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


#include <klib/out.h>

#define USE_READ_V1     false
#define USE_WRITE_V1    false
#define USE_UPDATE_V1   false
#define USE_BLOCK_V1    false
/* KReencFile and KEncryptFile need to use update v1 as it is different */
#define USE_VALIDATE_V1 false
#define USE_ISENC_V1    false

#define ENABLE_V2 true


/* ----------------------------------------------------------------------
 * KEncFileV1
 *   Base object class for the encryption, decryption and validation of
 *   the file format defined above
 */
typedef struct KEncFileV1  KEncFileV1;
#define KFILE_IMPL struct KEncFileV1
#include <kfs/impl.h>


#include "encfile-priv.h"

/* ----------
 * BufferCalcMD5
 *    Generate the MD5 digest for a buffer
 */
static
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


typedef struct KEncFileCiphers KEncFileCiphers;
struct KEncFileCiphers
{
    KCipher * master;
    KCipher * block;
};

typedef uint8_t KEncFileIVec [16];
/* -----
 */
struct KEncFileV1
{
    KFile dad;                  /* base class */
    KFile * encrypted;          /* encrypted file as a KFile */
    KEncFileCiphers ciphers;
    KEncFileBlock block;        /* */
    uint64_t encrypted_max;     /* highest read/written point in the encrypted file */
    KEncFileFooter foot;        /* contains rcrchecksum and blockcount */
#if ENABLE_V2
    KEncFileVersion version;
#endif
    bool dirty;                 /* data written but not flushed */
    bool bswap;
    bool eof;
    bool seekable;
    bool written;
    bool swarm;
};


/* ----------------------------------------------------------------------
 * Buffer - just some part of the encrypted file read or written
 *
 *   these functions exist to hide the potential multiple calls needed
 *   if the KFileRead or KFileWrite called on the encrypted file break up
 *   the requested amount into partial reads or writes
 */

/* ----------
 * BufferRead
 *    Fill a buffer with a requested number of bytes from the encrypted file.  
 *    Read either the requested number of bytes or up through EOF.
 *    The caller has to handle an EOF shorted buffer.
 */
static
rc_t KEncFileV1BufferRead (const KEncFileV1 * cself, uint64_t pos, void * buffer,
                         size_t bsize, size_t * pnum_read)
{
    KEncFileV1 * self;   /* for mutable fields */
    rc_t rc;

    assert (cself);
    assert (buffer);
    assert (pnum_read);

    *pnum_read = 0;

    self = (KEncFileV1*)cself; /* to hit mutable fields */

    /* we want to read a full requested size if possible so keep trying if we
     * haven't read enough yet. 
     * We can quit early only if EOF (i.e. read 0 bytes.
     */
    rc = KFileReadAll (self->encrypted, pos, buffer, bsize, pnum_read);
    if (rc)
        PLOGERR (klogErr,
                 (klogErr, rc,
                  "error reading position '$(P)' in encrypted file",
                  "P=%lu", pos));

    else if (self->encrypted_max < pos)
        self->encrypted_max = pos;
            
    return rc;
}


/* ----------
 * BufferWrite
 *    Write a buffer of requested size out to the encrypted file.
 *    return the number of bytes successfully written
 */
static
rc_t KEncFileV1BufferWrite (KEncFileV1 * self, uint64_t pos, const void * buffer,
                          size_t bsize, size_t * pnum_writ)
{
    rc_t rc;

    rc =  KFileWriteAll (self->encrypted, pos, buffer, bsize, pnum_writ);
    if ((rc == 0) && (self->encrypted_max < pos))
        self->encrypted_max = pos;

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

#define eByteOrderTag     (0x05031988)
#define eByteOrderReverse (0x88190305)

/* ----
 * Not unexpectedly the first version of this file will be "1"
 */
#if ENABLE_V2

#define eCurrentVersion (0x00000002)
#define eCurrentVersionReverse (0x02000000)

static
const KEncFileHeader const_header_v1
= { "NCBInenc", eByteOrderTag, 0x00000001 };

static
const KEncFileHeader const_bswap_header_v1
= { "NCBInenc", eByteOrderReverse, 0x01000000 };
    
#else

#define eCurrentVersion (0x00000001)
#define eCurrentVersionReverse (0x01000000)

#endif
    
static
const KEncFileHeader const_header
= { "NCBInenc", eByteOrderTag, eCurrentVersion };

static
const KEncFileHeader const_bswap_header
= { "NCBInenc", eByteOrderReverse, eCurrentVersionReverse };


/* ----------
 * HeaderRead
 *    Read the header of an encrypted file and validate it.
 */
static
rc_t KEncFileV1HeaderRead (KEncFileV1 * self)
{
    KEncFileHeader header;
    size_t num_read;
    rc_t rc;

    assert (self);

    rc = KEncFileV1BufferRead (self, 0, &header, sizeof (header), &num_read);
    if (rc)
        return rc;

    if (num_read != sizeof (header))
    {
        rc = RC (rcFS, rcFile, rcConstructing, rcHeader, rcTooShort);
        PLOGERR (klogErr,
                 (klogErr, rc, "error reading full header of encrypted "
                  "file  wanted '$(S)' got '$(N); bytes read", "S=%u,N=%u",
                  sizeof (header), num_read));
        return rc;
    }

    /* expected is encrypted on a system of the same endianess */
    if (memcmp (&header, &const_header, sizeof (header)) == 0)
    {
        self->bswap = false;
#if ENABLE_V2
        self->version = eCurrentVersion;
#endif
        return 0;
    }

    /* next most likely is encrypted on a machine of opposite endianess */
    if (memcmp (&header, &const_bswap_header, sizeof (header)) == 0)
    {
        self->bswap = true;
#if ENABLE_V2
        self->version = eCurrentVersion;
#endif
        return 0;
    }

#if ENABLE_V2
    if (memcmp (&header, &const_header_v1, sizeof (header)) == 0)
    {
        self->bswap = false;
        self->version = 1;
        return 0;
    }

    /* next most likely is encrypted on a machine of opposite endianess */
    if (memcmp (&header, &const_bswap_header_v1, sizeof (header)) == 0)
    {
        self->bswap = true;
        self->version = 1;
        return 0;
    }
#endif

    /* okay it's a "bad" header, so figure out why */
    if (memcmp (&header, &const_header, sizeof (header.file_sig)) != 0)
    {
        rc = RC (rcKrypto, rcFile, rcReading, rcHeader, rcInvalid);
        LOGERR (klogErr, rc, "file is not the NCBI encrypted file format");
        /* if this fails the rest doesn't matter */
        return rc;
    }

    switch (header.byte_order)
    {
    default:
        rc = RC (rcFS, rcFile, rcConstructing, rcByteOrder, rcInvalid);
        PLOGERR (klogErr, 
                 (klogErr, rc, "invalid byte order flag '$(F); in "
                  "encrypted file", "F=%X", header.byte_order));
        /* we don't return to possibly log other version errors */
        break;
    case eByteOrderReverse:
    case eByteOrderTag:
        break;
    }

    switch (header.version)
    {
    default:
        rc = RC (rcFS, rcFile, rcConstructing, rcHeader, rcBadVersion);
        PLOGERR (klogErr, 
                 (klogErr, rc, "can not decrypt version '$(V)'",
                  "V=%u", header.version));
        break;

    case 1:
#if ENABLE_V2
    case 2:
#endif
        break;
    }
    return rc;
}


/* -----
 * HeaderWrite
 *
 * build a ram copy of the header and write it to the file
 */
static
rc_t KEncFileV1HeaderWrite (KEncFileV1 * self)
{
    size_t num_writ;
    rc_t rc;

    rc = KEncFileV1BufferWrite (self, 0, &const_header, sizeof (const_header),
                              &num_writ);
    if (rc == 0)
    {
        if (num_writ != sizeof (const_header))
            rc = RC (rcFS, rcFile, rcWriting, rcHeader, rcInsufficient);
    }
    return rc;
}

LIB_EXPORT rc_t CC KEncFileV1WriteHeader_v1  (KFile * self)
{
    if (self == NULL)
        return RC (rcKrypto, rcFile, rcWriting, rcSelf, rcNull);
    return KEncFileV1HeaderWrite ((KEncFileV1*)self);
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
rc_t KEncFileV1FooterValidate (const KEncFileV1 * self,
                             uint64_t block_count, uint64_t crc_checksum)
{
    rc_t rc1 = 0, rc2 = 0;

    if (self->foot.block_count != block_count)
    {
        rc2 = RC (rcKrypto, rcFile, rcValidating, rcFile, rcCorrupt);
        LOGERR (klogErr, rc1, "bad block count in encrypted file footer");
    }
    if ((self->foot.crc_checksum != crc_checksum)
#if ENABLE_V2
        &&((crc_checksum != 0) || (self->version == 1))
#endif
        )
    {
        rc1 = RC (rcKrypto, rcFile, rcValidating, rcChecksum, rcCorrupt);
        LOGERR (klogErr, rc2, "bad crc checksum in encrypted file footer");
    }
    return (rc1 ? rc1 : rc2);
}


/* ----------
 * Write
 * when we write a footer we write from the values in the object.  They
 * are stored in the same object format as the footer so its very simple
 */
static
rc_t KEncFileV1FooterWrite (KEncFileV1 * self)
{
    KEncFileFooter foot;
    uint64_t offset;
    size_t num_writ;
    rc_t rc;

    memmove (&foot, &self->foot, sizeof (foot));
    if (self->bswap)
    {
        foot.block_count = bswap_64 (foot.block_count);
        foot.crc_checksum = bswap_64 (foot.crc_checksum);
    }

    offset = BlockId_to_CiphertextOffset ( self -> foot . block_count );

/*     assert ((self->encrypted_max == offset) || */
/*             (self->encrypted_max + sizeof(self->foot) == offset)); */

    rc = KEncFileV1BufferWrite (self, offset, &foot, sizeof (foot),
                              &num_writ);
    if (rc == 0)
    {
        if (num_writ != sizeof (foot))
            rc = RC (rcFS, rcFile, rcWriting, rcFile, rcInsufficient);
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
static
void KEncFileV1IVecInit (const KEncFileV1 * self, KEncFileIVec ivec)
{
    BufferCalcMD5 (&self->block.id, sizeof self->block.id, ivec);
}

 
/* ----------
 * BlockEncrypt
 */

static
rc_t KEncFileV1BlockEncrypt (KEncFileV1 * self, KEncFileBlock * e)
{
    SHA256State state;
    uint64_t id;
    uint16_t valid;
    uint16_t saved_valid;
    KEncFileCRC crc;
    uint8_t ivec [16];
    rc_t rc;

    assert (self);
    assert (e);

    /*
     * First we finish preparing the two ciphers by creating the block
     * user key out of the first part of the data and the shared Initialization
     * vector for Chained Block Cipher mode encryption out of the block id
     * 
     * create the initialization vector for this block
     */
    KEncFileV1IVecInit (self, ivec);

    /*
     * set the ivec for both the master and data block ciphers
     */
    rc = KCipherSetEncryptIVec (self->ciphers.master, ivec);
    if (rc)
        return rc;

    rc = KCipherSetEncryptIVec (self->ciphers.block, ivec);
    if (rc)
        return rc;

    /*
     * create the block user key out of the first 4kb of data and the block id 
     */
    saved_valid = valid = self->block.u.valid;
    id = self->block.id;

    SHA256StateInit (&state);
    SHA256StateAppend (&state, self->block.data, 
                       valid > 4096 ? 4096 : valid);
    SHA256StateAppend (&state, &id, sizeof (id));
    SHA256StateFinish (&state, self->block.key);
        
    /*
     * create the block key schedule out of the block user key
     */
    rc = KCipherSetEncryptKey (self->ciphers.block, self->block.key,
                               sizeof self->block.key);
    if (rc)
        return rc;

    /* 
     * Salt the block using the randomish user key to randomly select
     * data from the valid data.
     *
     * This will cover the data portion of the block past the last valid
     * byte.
     *
     * NOTE we are accessing a byte array as a word array.
     *
     * NOTE we are using the array named data to access data beyond it's end.
     */
    {
        uint16_t * pw;
        unsigned int windex;
        unsigned int rindex;
        size_t bindex;

        pw = (uint16_t*)self->block.key;
        windex = 0;

        for (bindex = valid;
             bindex < sizeof self->block.data + sizeof self->block.u; 
             ++ bindex)
        {
            /* this goes beyond the end of the data array by design */
            rindex = (size_t)pw[windex];
            rindex %= bindex;

            self->block.data[bindex] = self->block.data[rindex];
        
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
        id = bswap_64 (id);
        valid = bswap_16 (valid);
    }


    if (saved_valid == sizeof self->block.data)
        self->block.u.valid |= valid;
    else
        self->block.u.valid = valid;

    e->id = id;

    /* 
     * encrypt the block user key into the buffer
     */
    rc = KCipherEncryptCBC (self->ciphers.master, self->block.key, e->key,
                            sizeof (self->block.key) / sizeof (ivec));
    if (rc)
        return rc;

    /*
     * encrypt the data, offset and valid values
     */
    rc = KCipherEncryptCBC (self->ciphers.block, 
                            self->block.data, e->data,
                            (sizeof self->block.data + sizeof self->block.u) / sizeof (ivec));
    if (rc)
        return rc;

    self->block.u.valid = saved_valid;

    crc = CRC32 (0, e, (char*)(&e->crc)-(char*)e);

    self->block.crc = crc;

    if (self->bswap)
    {
        crc = bswap_32 (crc);
    }
    e->crc_copy = e->crc = crc;
    
    return 0;
}


/* ----------
 * BlockDecrypt
 *   decrypt decrypts the data from a KEncFileBlock into the KEncFileBlock
 *   in the KEncFile object
 */

static
rc_t KEncFileV1BlockDecrypt (KEncFileV1 * self, KEncFileBlock * e)
{
    uint8_t ivec [16];
    rc_t rc;

    assert (self);
    assert (e);

    /* create the initialization vector for this block */
    KEncFileV1IVecInit (self, ivec);

    /*
     * set the ivec for both the master and data block ciphers
     */
    rc = KCipherSetDecryptIVec (self->ciphers.master, ivec);
    if (rc)
        return rc;

    rc = KCipherSetDecryptIVec (self->ciphers.block, ivec);
    if (rc)
        return rc;

    /* 
     * decrypt the block key and initial vector using the user key and 
     * the computer ivec
     */
    rc = KCipherDecryptCBC (self->ciphers.master, e->key, self->block.key,
                            (sizeof e->key) / sizeof ivec);
    if (rc)
        return rc;

    /* 
     * now create the AES key for the block from the newly decrypted 
     * block key
     */
    rc = KCipherSetDecryptKey (self->ciphers.block, self->block.key,
                               sizeof self->block.key);
    if (rc)
        return rc;
    rc = KCipherDecryptCBC (self->ciphers.block, e->data, 
                            self->block.data, 
                            (sizeof e->data + sizeof e->u) / sizeof ivec);
    if (rc)
        return rc;

    if (self->bswap)
    {
        self->block.u.valid = bswap_16 (self->block.u.valid);
/*         self->block.id = bswap_64 (self->block.id); */
    }

    /* we choose not to flag errors in decryption so don't validate these */
    if (self->block.u.valid & 0x8000)
        self->block.u.valid = 0x8000;

    return rc;
}


/* ----------
 * BlockRead
 *    read a requested block from the encrypted file; 
 *    decryption is a separate step
 */
static
rc_t KEncFileV1BlockRead (const KEncFileV1 * cself, uint64_t block_id,
                        KEncFileBlock * block, bool validate)
{
    KEncFileV1 * self;    /* mutable fields */
    uint64_t pos;
    uint64_t max;
    size_t num_read;
    rc_t rc;
    KEncFileBlock e;

    assert (cself);
    assert (block);

    self = (KEncFileV1*)cself;
    pos = BlockId_to_CiphertextOffset ( block_id );

    /* set aside the current maximum position within the encrypted file */
    max = self->encrypted_max;

    memset (&self->block, 0, sizeof self->block);
    self->block.id = block_id;
    self->block.u.valid = 0;
    self->dirty = false;

    rc = KEncFileV1BufferRead (self, pos, &e, sizeof e, &num_read);
    if (rc)
        PLOGERR (klogErr,
                 (klogErr, rc,
                  "error in reading block '$(BID)' from encrypted file",
                  "BID=%lu", block_id));
    else
    {
        uint32_t crc;

        /* we'll judge the success of the read now on how many bytes read
         * there are only three legitimate values:  
         *   + size of a block 
         *   + size of a footer
         *   + zero meaning no footer yet
         *
         * This might have to change to handled streamed files where the 
         * producer jsut hasn't finished yet
         */
        switch (num_read)
        {
        default: /* bad value means a truncated file most likely */
            rc = RC (rcKrypto, rcFile, rcReading, rcFile, rcInsufficient);
            PLOGERR (klogErr,
                     (klogErr, rc, "wrong size reading block '$(B)' at '$(P)' "
                      "from encrypted file - likely truncated", "B=%lu,P=%lu",
                      block_id, pos));
            break;

        case sizeof e:
            if (max <= pos) /* if we hadn't read this far do accounting */
            {
                if (self->bswap)
                {
                    e.crc = bswap_32 (e.crc);
                    e.id = bswap_64 (e.id);
                }

                if (block_id != e.id)
                {
                    PLOGERR (klogErr,
                             (klogErr,
                              RC (rcKrypto, rcFile, rcValidating, rcIndex, rcIncorrect),
                              "error validating id for block '$(BID)' is not $(C2)", 
                              "BID=%lu,C2=%lu", block_id, e.id));
                }

                if (validate)
                {
                    crc = CRC32 (0, &e, (char*)(&e.crc)-(char*)&e);


                    if (crc != e.crc)
                    {
                        PLOGERR (klogErr,
                                 (klogErr,
                                  RC (rcKrypto, rcFile, rcValidating, rcCrc, rcCorrupt),
                                  "error validating crc for block '$(BID)' $(C1) is not $(C2)", 
                                  "BID=%lu,C1=0x%X,C2=0x%X", block_id,
                                  crc, e.crc));
                    }
                }

                ++self->foot.block_count;
/* Byte Endian problem! */
                self->foot.crc_checksum += e.crc;
            }
            memmove (block, &e, sizeof e);
            self->eof = false;
            break;

        case sizeof (self->foot):
            {
                KEncFileFooter foot;
                memmove (&foot, &e, sizeof foot);
                if (self->bswap)
                {
                    foot.block_count = bswap_64 (foot.block_count);
                    foot.crc_checksum = bswap_64 (foot.crc_checksum);
                }
                if (validate && !self->written)
                {
                    rc = KEncFileV1FooterValidate (self, foot.block_count,
                                                 foot.crc_checksum);
                    if (rc)
                        break;
                }
            }
            /* fall through */
        case 0:
            self->block.u.valid = 0;
            self->block.crc = 0;
            self->eof = true;
            return 0;

        }
    }
    return rc;
}

static
rc_t KEncFileV1BlockWrite (KEncFileV1 * self)
{
    KEncFileBlock e;
    uint64_t block_offset;
    size_t num_writ;
    rc_t rc;
 
    assert (self);

    if (self->block.u.valid > sizeof self->block.data)
        return RC (rcFS, rcFile, rcWriting, rcBuffer, rcCorrupt);

    /* where in the file is this block */
    block_offset = BlockId_to_CiphertextOffset ( self -> block . id );

    /* if this is an update to a block take out the old crc value */
    if (block_offset < self->encrypted_max)
        self->foot.crc_checksum -= self->block.crc;

    /* if it's not we need to add it as a new block in the count */
    else
        ++self->foot.block_count;

    rc = KEncFileV1BlockEncrypt (self, &e);
    if (rc)
        return rc;

/* Byte Endian problem */
    self->foot.crc_checksum += self->block.crc;

    /* now write the buffer to the encrypted file */
    rc = KEncFileV1BufferWrite (self, block_offset, &e, sizeof e, &num_writ);

    if ((rc == 0) && (num_writ != sizeof e))
    {
        rc = RC (rcFS, rcFile, rcWriting, rcFile, rcTooShort);
        PLOGERR (klogErr,
                 (klogErr, rc, "incomplete block write '$(B)' wanted"
                  " '$(V)' got '$(N)'", "B=%lu,V=%u,N=%u",
                  self->foot.block_count+1, self->block.u.valid, num_writ));
    }
    return rc;
}


static
rc_t KEncFileV1BlockFlush (KEncFileV1 *self)
{
    rc_t rc = 0;

    assert (self);
    assert (self->block.u.valid);
    assert (self->block.u.valid <= sizeof (self->block.data));

    /* first do we need to deal with the file header? 
     * if the encrypted file has not been written to we do */
    if ((self->encrypted_max == 0) && (self->swarm == false))
    {
        rc = KEncFileV1HeaderWrite (self);
        if (rc)
            return rc;
    }

    if (self->dirty)
    {
        rc = KEncFileV1BlockWrite (self);
        if (rc == 0)
            self->dirty = false;
    }
    return rc;
}


/*
 * block_id - which block to read and decrypt
 * fill - if not false, we fill in zero blocks up through this block
 *
 */
static
rc_t KEncFileV1BlockSeek (KEncFileV1 * self, uint64_t block_id, bool fill, bool validate)
{
    KEncFileBlock b;
    rc_t rc;

    if (block_id != self->block.id)
    {
        if (self->dirty)
        {
            /* flush what we got */
            rc = KEncFileV1BlockFlush (self);
            if (rc)
                return rc;
        }

        if (validate && (block_id > self->block.id))
        {
            uint64_t tid;

            for (tid = self->block.id + 1; tid < block_id; ++tid)
            {
                if ( self -> encrypted_max > BlockId_to_CiphertextOffset ( block_id ) )
                    continue;

                memset (&b, 0, sizeof b);

                rc = KEncFileV1BlockRead (self, tid, &b, validate);
                if (rc)
                    return rc;

                if (self->eof)
                {
                    if (fill)
                    {
                        self->eof = false;
                        memset (self->block.data, 0, sizeof (self->block.data));
                        self->block.u.valid = sizeof (self->block.data);
                        self->dirty = true;
                        rc = KEncFileV1BlockFlush (self);
                        if (rc)
                            return rc;
                    }
                    else
                        return 0;
                }
                /* in this loop a less than full block is end of file */
                else if (fill)
                {
                    rc = KEncFileV1BlockDecrypt (self, &b);
                    if (rc)
                        return rc;

                    if (self->block.u.valid < sizeof self->block.u.valid)
                    {
                        memset (self->block.data + self->block.u.valid, 0, 
                                sizeof (self->block.data) - self->block.u.valid);
                        self->dirty = true;
                        rc = KEncFileV1BlockFlush (self);
                        if (rc)
                            return rc;
                    }
                    else
                    {
                        return 0;
                    }
                }
            }
        }
    }
    /* done with intervening blocks */
    memset (&b, 0, sizeof b);

    rc = KEncFileV1BlockRead (self, block_id, &b, validate);
    if (rc)
        return rc;

    if (! self->eof)
    {
        rc = KEncFileV1BlockDecrypt (self, &b);
    }
    if (fill)
    {
        self->block.id = block_id;
        self->block.u.valid = 0;
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
rc_t CC KEncFileV1DestroyRead (KEncFileV1 *self)
{
    rc_t rc1 = 0;
    rc_t rc2 = 0;
    rc_t rc3 = 0;

    if (self)
    {
        rc1 = KFileRelease (self->encrypted);
        rc2 = KCipherRelease (self->ciphers.master);
        rc3 = KCipherRelease (self->ciphers.block);
        free (self);
        return rc1 ?  rc1 : rc2 ? rc2 : rc3;
    }
    return 0;
}


static
rc_t CC KEncFileV1DestroyWrite (KEncFileV1 *self)
{
    rc_t rc1;
    rc_t rc2;

    rc1 = (self->block.u.valid) ? KEncFileV1BlockFlush (self) : 0;
    if ((rc1 == 0)&&(self->encrypted_max != 0))
        rc1 = KEncFileV1FooterWrite (self);
    rc2 = KFileRelease (self->encrypted);

    return (rc1 ?  rc1 : rc2);
}


static
rc_t CC KEncFileV1DestroySwarm (KEncFileV1 *self)
{
    rc_t rc1 = 0;
    rc_t rc2 = 0;
    rc_t rc3 = 0;

    if (self)
    {
        rc1 = KFileRelease (self->encrypted);
        rc2 = KCipherRelease (self->ciphers.master);
        rc3 = KCipherRelease (self->ciphers.block);
        free (self);
        return rc1 ?  rc1 : rc2 ? rc2 : rc3;
    }
    return 0;
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
struct KSysFile *CC KEncFileV1GetSysFileUnsupported (const KEncFileV1 *self, uint64_t *offset)
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
rc_t CC KEncFileV1RandomAccess (const KEncFileV1 *self)
{
    assert (self != NULL);
    assert (self->encrypted != NULL);
    return KFileRandomAccess (self->encrypted);
}


static
rc_t CC KEncFileV1RandomAccessUnsupported (const KEncFileV1 *self)
{
    return RC ( rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported );
}


/* ----------------------------------------------------------------------
 * Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t CC KEncFileV1SizeUnsupported (const KEncFileV1 *self, uint64_t *size)
{
    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}


/* ----------------------------------------------------------------------
 * SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KEncFileV1SetSizeUnsupported (KEncFileV1 *self, uint64_t size)
{
    assert (self);
    return RC ( rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported );
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
rc_t CC KEncFileV1ReadUnsupported (const KEncFileV1 *self,
                                 uint64_t pos,
                                 void *buffer,
                                 size_t bsize,
                                 size_t *num_read)
{
    return RC ( rcFS, rcFile, rcReading, rcFunction, rcUnsupported );
}


static
rc_t CC KEncFileV1Read	(const KEncFileV1 *cself,
                         uint64_t pos,
                         void *buffer,
                         size_t bsize,
                         size_t *num_read)
{
    KEncFileV1 * self = (KEncFileV1 *)cself; /* mutable values */
    uint64_t block_id;
    uint32_t offset;
    size_t to_copy;
    rc_t rc = 0;

    assert (self); /* checked in file.c KFileRead() */
    /* pos can be 'anything' */
    assert (buffer); /* checked in file.c KFileRead() */
    assert (bsize); /* checked in file.c KFileRead() */
    assert (num_read); /* checked in file.c KFileRead() */

    *num_read = 0;

    block_id = PlaintextOffset_to_BlockId (pos, &offset);

    /*
     * are we on the wrong block?
     * Or are do we need to read the first block?
     */
    if ((block_id != self->block.id) || (self->block.u.valid == 0))
    {
        rc = KEncFileV1BlockSeek (self, block_id, false, false);
        if (rc)
            return rc;
    }

    /*
     * if we are trying to read past the end of the file 
     * return 0 with nothing read
     */
    if (self->eof || (block_id != self->block.id) ||
        (offset >= self->block.u.valid))
        return 0;

    to_copy = self->block.u.valid - offset;
    if (to_copy > bsize)
        to_copy = bsize;

    memmove (buffer, self->block.data + offset, to_copy);
    *num_read = to_copy;

    return 0;
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
 *
 * Unsupported as we now treat archives as READ ONLY
 */
static
rc_t CC KEncFileV1WriteUnsupported (KEncFileV1 *self, uint64_t pos,
                                  const void *buffer, size_t bsize,
                                  size_t *num_writ)
{
    rc_t rc = RC (rcFS, rcFile, rcReading, rcFunction, rcUnsupported);
    LOGERR (klogInt, rc, "KFileRead failed to filter call");
    return rc;
}

static
rc_t KEncFileV1WriteInt (KEncFileV1 *self, uint64_t block_id, uint32_t block_offset,
                       const void *buffer, size_t bsize,
                       size_t *pnum_writ, bool seek)
{
    const uint8_t * pb;
    size_t tot_writ;
    rc_t rc;

    assert (self);
    assert (self->block.u.valid <= sizeof (self->block.data));
    assert (block_offset < sizeof (self->block.data));
    assert (buffer);
    assert (bsize);
    assert (pnum_writ);

    if (seek)
    {
        /* if we need to change blocks */
        if (block_id != self->block.id)
        {
            rc = KEncFileV1BlockSeek (self, block_id, true, false);
            if (rc)
                return rc;
        }
        /* do we have to fill some of this block */
        if (self->block.u.valid < block_id)
        {
            memset (self->block.data + self->block.u.valid, 0,
                    block_offset - self->block.u.valid);
            self->block.u.valid = block_offset;
        }
    }

    /*
     * is the full write within this block?
     * doesn't matter whether we are skipping back or not
     */
    if ((block_offset + bsize) <= sizeof (self->block.data))
    {
        memmove (self->block.data + block_offset, buffer, bsize);
        self->dirty = true;
        if (block_offset + bsize > self->block.u.valid)
            self->block.u.valid = (KEncFileBlockValid) ( block_offset + bsize );
        *pnum_writ = bsize;
        return 0;
    }

    /* we are either forcing ourselves backward or not */
    else 
        self->block.u.valid = block_offset;

    /* by this point we are 'at' the right place in the file */
    /* do we have an existing (possibly partial) block? */

    /* change to a sized target pointer */
    pb = buffer;
    tot_writ = 0;

    /* handle this first block with some data in it */
    if (self->block.u.valid != 0)
    {
        size_t remaining;

        /* how many bytes left in this block */
        remaining = sizeof self->block.data - self->block.u.valid;

        memmove (self->block.data + self->block.u.valid, buffer, remaining);
        self->block.u.valid = sizeof self->block.data;

        self->dirty = true;
        rc = KEncFileV1BlockFlush (self);
        if (rc)
        {
            LOGERR (klogErr, rc, "error flushing block in encrypt");
            return rc;
        }

        tot_writ = remaining;
        pb += remaining;
        bsize -= remaining; 
        ++block_id;
    }

    /* now loop through complete blocks to write */
    for (; bsize >= sizeof self->block.data; bsize -= sizeof self->block.data)
    {
        memmove (self->block.data, pb, sizeof self->block.data);

        self->block.u.valid = sizeof self->block.data;
        self->block.id = block_id;
        self->dirty = true;
        rc = KEncFileV1BlockFlush (self);
        if (rc)
        {
            LOGERR (klogErr, rc, "error flushing block in encrypt");
            return rc;
        }
        pb += sizeof self->block.data;
	tot_writ += sizeof self->block.data;
        ++block_id;
    }

    /* now any last partial block that starts at the beginning of a block */
    if (bsize != 0)
    {
        KEncFileBlock b;

        if ( self -> encrypted_max > BlockId_to_CiphertextOffset ( block_id ) )
        {
            rc = KEncFileV1BlockRead (self, block_id, &b, false);
            if (rc)
                return rc;
            /* if valid is not 0 we actually read something */
            if (b.u.valid)
                rc = KEncFileV1BlockDecrypt (self, &b);
        }
        else
        {
            self->block.u.valid = 0;
            self->block.id = block_id;
        }

        memmove (self->block.data, pb, bsize);

        if (bsize > self->block.u.valid)
            self->block.u.valid = (KEncFileBlockValid) bsize;

        self->dirty = true;
        tot_writ += bsize;
    }
    *pnum_writ = tot_writ;
    return 0;
}


static
rc_t CC KEncFileV1Write (KEncFileV1 *self, uint64_t pos,
                       const void *buffer, size_t bsize,
                       size_t *pnum_writ)
{
    uint64_t block_id;
    uint64_t curr_block_id;
    uint32_t block_offset;
    uint32_t curr_block_offset;
    rc_t rc = 0;

    /* -----
     * check for incorrect call with a seek attempt
     * we only allow seeks on read or read/write files
     *
     * find our location in the encrypted file by block id
     * and offset
     */
    block_id = PlaintextOffset_to_BlockId (pos, &block_offset);
    curr_block_id = PlaintextOffset_to_BlockId
        ( BlockId_to_PlaintextOffset ( self -> block . id ) + self -> block . u . valid,
         &curr_block_offset);

    /* are we writing to the wrong block/offset? */
    if ((block_id != curr_block_id) || (block_offset != curr_block_offset))
    {
        rc = RC (rcFS, rcFile, rcWriting, rcOffset, rcIncorrect);
        PLOGERR (klogErr, (klogErr, rc, "attempt to seek in encryption write at"
                           " '$(O)' seek to '$(P)'", "O=%lu,P=%lu",
                           BlockId_to_CiphertextOffset ( self -> block . id ), pos ) );
    }
    else
    {
        if ((block_id != 0) && (block_id != self->block.id))
        {
            rc = KEncFileV1BlockFlush (self);
            if (rc == 0)
            {
                ++self->block.id;
                self->block.u.valid = 0;
            }
        }
        
        if (rc == 0)
            rc = KEncFileV1WriteInt (self, block_id, block_offset, buffer,
                                       bsize, pnum_writ, false);
    }
    return rc;
}


static
rc_t CC KEncFileV1WriteSwarm (KEncFileV1 *self, uint64_t pos,
                            const void *buffer, size_t bsize,
                            size_t *pnum_writ)
{
    uint32_t block_offset;
    rc_t rc;

    self->block.id = PlaintextOffset_to_BlockId (pos, &block_offset);
    self->block.u.valid = 0;
    if (bsize > sizeof self->block.data - block_offset)
        bsize = sizeof self->block.data - block_offset;

    rc = KEncFileV1WriteInt (self, self->block.id, block_offset, buffer,
                           bsize, pnum_writ, false);
    if (rc == 0)
        rc = KEncFileV1BlockFlush (self);
    return rc;
}


/* ----------------------------------------------------------------------
 * Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KEncFileV1Type (const KEncFileV1 *self)
{
    assert (self != NULL);
    assert (self->encrypted != NULL);

    return KFileType (self->encrypted);
}


/* ----------------------------------------------------------------------
 * KEncFileV1Make
 *  create a new file object
 */

/* ----------
 * KeysInit
 */
static
rc_t KEncFileV1CiphersInit (KEncFileV1 * self, const KKey * key, bool read, bool write)
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



/* ----------
 * MakeInt
 *    common make for all encryptor/decryptors
 */
static
rc_t KEncFileV1MakeInt (KEncFileV1 ** pself, KFile * encrypted,
                      const KFile_vt_v1 * vt, bool r, bool w, bool v)
{
    KEncFileV1 * self;
    rc_t rc;

    assert (pself);
    assert (encrypted);

    /* allocate and zero out an object since we want much of it to be zeroed */
    self = calloc (sizeof *self, 1);
    if (self == NULL)
    {
        rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
        LOGERR (klogSys, rc,
                "out of memory creating encrypter and/or decryptor");
    }
    else
    {
        /* all KFiles get this initialization */
        rc = KFileInit (&self->dad, (const KFile_vt*)vt, "KEncFileV1", "no-name", r, w);
        if (rc)
            LOGERR (klogInt, rc, "failure initialize encrypted file root class");
        else
        {
            rc = KFileAddRef (encrypted);
            if (rc == 0)
            {
                self->encrypted = encrypted;
                *pself = self;
                return 0;
            }
        }
        free (self);
    }
    return rc;
}


/* ----------
 * MakeCmn
 * common parameter validation for all encryptor/decryptors
 */
static
rc_t KEncFileV1MakeCmn (KEncFileV1 ** pself, KFile * encrypted, const KKey * key,
                      const KFile_vt_v1 * vt, bool r, bool w)
{
    rc_t rc = 0;

    do
    {
        CRC32Init();    /* likely to be called way too often */

        if (pself == NULL)
        {
            rc = RC (rcFS, rcFile, rcConstructing, rcSelf, rcNull);
            LOGERR (klogErr, rc,
                    "pointer to self NULL when creating "
                    "an encryptor/decryptor");
            break;
        }
        *pself = NULL;

        if (encrypted == NULL)
        {
            rc = RC (rcFS, rcFile, rcConstructing, rcParam, rcNull);
            LOGERR (klogErr, rc, 
                    "encrypted file not supplied when creating "
                    "an encryptor/decryptor");
            break;
        }

        if (key == NULL)
        {
            rc = RC (rcFS, rcFile, rcConstructing, rcParam, rcNull);
            LOGERR (klogErr, rc, 
                    "key not supplied when creating an encryptor/decryptor");
            break;
        }

        switch (key->type)
        {
        default:
            rc = RC (rcFS, rcFile, rcConstructing, rcParam, rcInvalid);
            PLOGERR (klogErr,
                     (klogErr, rc, "invalid key type '$(T)' should be "
                      "kkeyAES128(1), kkeyAES192(2) or kkeyAES256(3)",
                      "T=%u", key->type));
            break;

        case kkeyAES128:
        case kkeyAES192:
        case kkeyAES256:
            break;
        }
        if (rc == 0)
        {
            KEncFileV1 * self;

            assert (vt);
            assert ((r == true) || (r == false));
            assert ((w == true) || (w == false));

            rc = KEncFileV1MakeInt (&self, encrypted, vt, r, w, false);
            if (rc == 0)
            {
                rc = KEncFileV1CiphersInit (self, key, r, w);
                if (rc == 0)
                {
                    *pself = self;
                    return 0;
                }
                KFileRelease (&self->dad);
            }
        }
    } while (0);
    return rc;
}



static const KFile_vt_v1 vtKEncFileV1Read =
{
    /* version */
    1, 1,

    /* 1.0 */
    KEncFileV1DestroyRead,
    KEncFileV1GetSysFileUnsupported,
    KEncFileV1RandomAccess,
    KEncFileV1SizeUnsupported,
    KEncFileV1SetSizeUnsupported,
    KEncFileV1Read,
    KEncFileV1WriteUnsupported,

    /* 1.1 */
    KEncFileV1Type
};
static const KFile_vt_v1 vtKEncFileV1Write =
{
    /* version */
    1, 1,

    /* 1.0 */
    KEncFileV1DestroyWrite,
    KEncFileV1GetSysFileUnsupported,
    KEncFileV1RandomAccessUnsupported,
    KEncFileV1SizeUnsupported,
    KEncFileV1SetSizeUnsupported,
    KEncFileV1ReadUnsupported,
    KEncFileV1Write,

    /* 1.1 */
    KEncFileV1Type
};
static const KFile_vt_v1 vtKEncFileV1Update =
{
    /* version */
    1, 1,

    /* 1.0 */
    KEncFileV1DestroySwarm,
    KEncFileV1GetSysFileUnsupported,
    KEncFileV1RandomAccess,
    KEncFileV1SizeUnsupported,
    KEncFileV1SetSizeUnsupported,
    KEncFileV1ReadUnsupported,
    KEncFileV1WriteSwarm,

    /* 1.1 */
    KEncFileV1Type
};
static const KFile_vt_v1 vtKEncFileV1Validate =
{
    /* version */
    1, 1,

    /* 1.0 */
    KEncFileV1DestroyRead,
    KEncFileV1GetSysFileUnsupported,
    KEncFileV1RandomAccessUnsupported,
    KEncFileV1SizeUnsupported,
    KEncFileV1SetSizeUnsupported,
    KEncFileV1ReadUnsupported,
    KEncFileV1WriteUnsupported,

    /* 1.1 */
    KEncFileV1Type
};


/* ----------
 * Read mode is fully seekable if the underlying KFile is seekable some
 * integrity checking will not be performed in allowing this seeking.
 */
LIB_EXPORT rc_t CC KEncFileMakeRead_v1 (const KFile ** pself, 
                                        const KFile * encrypted,
                                        const KKey * key)
{
    KEncFileV1 * self;
    rc_t rc;
    rc = KEncFileV1MakeCmn (&self, (KFile *)encrypted, key, &vtKEncFileV1Read, 
                          true, false);
    if (rc)
        LOGERR (klogErr, rc, "error constructing decryptor");
    else
    {
        rc = KEncFileV1HeaderRead (self);
        if (rc)
            LOGERR (klogErr, rc, "error reading encrypted file header");
        else
        {
            *pself = &self->dad;
            return 0;
        }
        KFileRelease (&self->dad);
    }
    return rc;
}


/* ----------
 * Write mode encrypted file can only be written straight through form the
 * first byte to the last.
 */
LIB_EXPORT rc_t CC KEncFileMakeWrite_v1 (KFile ** pself, 
                                         KFile * encrypted,
                                         const KKey * key)
{
    KEncFileV1 * self;
    rc_t rc;

    rc = KFileSetSize (encrypted, 0);
#if 0
    if (rc)
        LOGERR (klogWarn, rc, "error truncating output file - "
                "corrupted file might result");
#endif

    rc = KEncFileV1MakeCmn (&self, encrypted, key, &vtKEncFileV1Write, 
                          false, true);
    if (rc)
        LOGERR (klogErr, rc, "error creating encryptor");
    else
        *pself = &self->dad;
    return rc;
}


/* ----------
 * Update mode is read/write mode where seeking within the file is allowed.
 */


LIB_EXPORT rc_t CC KEncFileV1MakeUpdate_v1 (KFile ** pself, 
                                          KFile * encrypted,
                                          const KKey * key)
{
    KEncFileV1 * self;
    rc_t rc;

    rc = KEncFileV1MakeCmn (&self, encrypted, key, &vtKEncFileV1Update, 
                          false, true);
    if (rc)
        LOGERR (klogErr, rc, "error creating encryptor");
    else
    {
        self->swarm = true;
        *pself = &self->dad;
    }
    return rc;
}


/* ----------
 * Validate mode is useful only for the KFileEncValidate function
 */
static
rc_t KEncFileV1MakeValidate (const KEncFileV1 ** pself, const KFile * encrypted)
{
    KEncFileV1 * self;
    rc_t rc;

    assert (pself);
    assert (encrypted);

    rc = KEncFileV1MakeInt (&self, (KFile*)encrypted, &vtKEncFileV1Validate, true, false, true);
    if (rc)
        LOGERR (klogErr, rc, "error making KEncFileV1");
    else
    {
        rc = KEncFileV1HeaderRead (self);
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


/* ----------
 * Validate mode can not be read or written.
 * Upon open the whole file is read from begining to end and all CRC
 * and other integrity checks are performed immedaitely
 */
LIB_EXPORT rc_t CC KEncFileV1Validate_v1 (const KFile * encrypted)
{
    const KEncFileV1 * file;
    rc_t rc;

    /* fail if a NULL parameter: can't validate all addresses */
    if (encrypted == NULL)
    {
        rc = RC (rcKrypto, rcFile, rcValidating, rcParam, rcNull);
        LOGERR (klogErr, rc, "encrypted file was null when trying to validate");
        return rc;
    }

    /* if the parameter is already a KEncFileV1 work on the file behind it instead */
    /* we definitely can't work on a file being written that has not been closed */
    if (encrypted->vt == (const KFile_vt*)&vtKEncFileV1Read)
        encrypted = (const KFile *)(((const KEncFileV1*)encrypted)->encrypted);

    /* file header is validated within the call to Make Validate */
    rc = KEncFileV1MakeValidate (&file, encrypted);
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
            KEncFileBlock block;

            STSMSG (2, ("reading block '%u' at '%lu'", block_count,
                        BlockId_to_CiphertextOffset ( block_count ) ) );
           
            rc = KEncFileV1BlockRead (file, block_count, &block, true);
            if (rc)
            {
                STSMSG (2, ("read error at block '%u'", block_count));
                break;
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


LIB_EXPORT rc_t CC KFileIsEnc_v1 (const char * buffer, size_t buffer_size)
{
    const KEncFileHeader * header;

    if ((buffer == NULL) || (buffer_size == 0))
        return RC  (rcFS, rcFile, rcIdentifying, rcParam, rcNull); 

    header = (const KEncFileHeader*)buffer;

    if (buffer_size < sizeof header->file_sig)
        return RC (rcFS, rcFile, rcIdentifying, rcBuffer, rcInsufficient); 

    if (buffer_size > sizeof (*header))
        buffer_size = sizeof (*header);

    if ((memcmp (header, &const_header, buffer_size) == 0) ||
        (memcmp (header, &const_bswap_header, buffer_size) == 0)
#if ENABLE_V2
        ||
        (memcmp (header, &const_header_v1, buffer_size) == 0) ||
        (memcmp (header, &const_bswap_header_v1, buffer_size) == 0)
#endif
        )
        return 0;

    return RC (rcFS, rcFile, rcIdentifying, rcFile, rcWrongType); 
}



/* ----------
 * Read mode is fully seekable if the underlying KFile is seekable some
 * integrity checking will not be performed in allowing this seeking.
 */
LIB_EXPORT rc_t CC KEncFileMakeRead (const struct KFile ** pself,
                                        const struct KFile * encrypted_input,
                                        const struct KKey * key)
{
#if USE_READ_V1
    return KEncFileV1MakeRead_v1 (pself, encrypted_input, key);
#else
    return KEncFileMakeRead_v2 (pself, encrypted_input, key);
#endif
}


/* ----------
 * Write mode encrypted file can only be written straight through from the
 * first byte to the last.
 */
LIB_EXPORT rc_t CC KEncFileMakeWrite (struct KFile ** pself,
                                         struct KFile * encrypted_output,
                                         const struct KKey * key)
{
#if USE_WRITE_V1
    return KEncFileV1MakeWrite_v1 (pself, encrypted_output, key);
#else
    return KEncFileMakeWrite_v2 (pself, encrypted_output, key);
#endif
}

/* ----------
 * Update mode is read/write mode where seeking within the file is allowed.
 *
 * NOTE this is in the private interface because it is not actually working
 * yet.
 */
LIB_EXPORT rc_t CC KEncFileMakeUpdate (struct KFile ** pself, 
                                       struct KFile * encrypted,
                                       const struct KKey * key)
{
#if USE_UPDATE_V1
    return KEncFileV1MakeUpdate_v1 (pself, encrypted, key);
#else
    return KEncFileMakeUpdate_v2 (pself, encrypted, key);
#endif
}
LIB_EXPORT rc_t CC KEncFileMakeWriteBlock (struct KFile ** pself, 
                                           struct KFile * encrypted,
                                           const struct KKey * key)
{
#if USE_BLOCK_V1
    return KEncFileV1MakeUpdate_v1 (pself, encrypted, key);
#else
    return KEncFileMakeBlock_v2 (pself, encrypted, key);
#endif
}


LIB_EXPORT rc_t CC KEncFileWriteHeader  (KFile * self)
{
#if USE_BLOCK_V1
    return KEncFileV1WriteHeader_v1  (self);
#else
    return KEncFileWriteHeader_v2  (self);
#endif
}


/* ----------
 * Validate mode can not be read or written.
 * Upon open the whole file is read from begining to end and all CRC
 * and other integrity checks are performed immedaitely
 */
LIB_EXPORT rc_t CC KEncFileValidate (const struct KFile * encrypted)
{
#if USE_VALIDATE_V1
    return KEncFileV1Validate_v1 (encrypted);
#else
    return KEncFileValidate_v2 (encrypted);
#endif
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
LIB_EXPORT rc_t CC KFileIsEnc (const char * buffer, size_t buffer_size)
{
#if USE_ISENC_V1
    return KFileIsEnc_v1 (buffer, buffer_size);
#else
    return KFileIsEnc_v2 (buffer, buffer_size);
#endif
}



#ifdef __cplusplus
}
#endif



/* end of file encfile.c */

