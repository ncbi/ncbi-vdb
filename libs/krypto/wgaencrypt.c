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
#include <krypto/cipher.h>
#include <krypto/ciphermgr.h>

#include <krypto/wgaencrypt.h>

#include <kfs/file.h>
#include <kfs/countfile.h>
#include <kfs/md5.h>
#include <kfs/buffile.h>
#include <kfs/nullfile.h>

#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/checksum.h>
#include <klib/log.h>
#include <klib/debug.h>

#include <sysalloc.h>

#include <strtol.h>

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <os-native.h> /* strncasecmp on Windows */

/* retain errors in encryption to be compatible with C++ */
#define RETAINED_COMPATIBILTY_WITH_ERROR 1


#define WGA_AES_BITS (256)
#define ECB_BYTES (16)
#define DEFAULT_BUFF_SIZE       (32*1024)


#define DEBUG_STS(msg)     DBGMSG(DBG_KRYPTO,DBG_FLAG(DBG_KRYPTO_STS),msg)
#define DEBUG_CFG(msg)     DBGMSG(DBG_KRYPTO,DBG_FLAG(DBG_KRYPTO_CFG_,msg)
#define DEBUG_ENCRYPT(msg) DBGMSG(DBG_KRYPTO,DBG_FLAG(DBG_KRYPTO_ENCRYPT_,msg)
#define DEBUG_DECRYPT(msg) DBGMSG(DBG_KRYPTO,DBG_FLAG(DBG_KRYPTO_DECRYPT_,msg)

static
void CalcMD5 (void * buffer, size_t size, char hexstr [32])
{
    static const char to_hex[16] = "0123456789abcdef";
    MD5State state;
    uint8_t digest [16];
    uint8_t * bin;
    char * hex;
    int ix;

    MD5StateInit (&state);
    MD5StateAppend (&state, buffer, size);
    MD5StateFinish (&state, digest);

    bin = digest;
    hex = hexstr;

    for (ix = 0; ix < sizeof digest; ++ix)
    {
        uint8_t upper;
        uint8_t lower;

        upper = *bin++;
        lower = upper & 0xF;
        upper >>= 4;

        *hex++ = to_hex [upper];
        *hex++ = to_hex[lower];
    }
}


/* ----------------------------------------------------------------------
 * KWGAEncFileMagic is extracted from 
 *      internal/WGA/access_countrol/src/wga/fuse/enc_reader.[ch]pp
 * 
 * the size 9 is not commented on but appears to be the size of ASCIZ
 * that is the 'magic' pattern including the NUL.
 *
 * There is no version number in the header and that NUL could be turned
 * into a single byte version.
 */
typedef char KWGAEncFileMagic [9];
static const KWGAEncFileMagic ncbi_crypt_magic = "NeCnBcIo";

/* ----------------------------------------------------------------------
 * Some values are stored in the encrypted file header using an odd radix 33
 */
#define KWGA_ENC_FILE_HEADER_RADIX      (33)

#if 0
static
const char KWGAEncFileHeaderAlphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
#endif

/* ----------------------------------------------------------------------
 * KWGAEncFileHeader is extracted from 
 *      internal/WGA/access_countrol/src/wga/fuse/enc_reader.[ch]pp
 *
 * There is no version number in this header; see the comment about KWGAEncFileMagic
 */
typedef uint8_t FER_ENCODING;
enum
{
    fer_encDES,
    fer_encBLF,
    fer_encAES
};

typedef struct KWGAEncFileHeader
{
    char magic [9];     /* "NeCnBcIo" -- NUL could be turned into a version */          /* 00 */
    char block_sz [8];  /* radixx33 encoded: block size in bytes / usually 4096  */     /* 09 */
    char file_sz [16];  /* radixx33 encoded: total size of unencrypted file */          /* 11 */
    char mtime [8];     /* radixx33 encoded: 32 bit time_t  */                          /* 21 */
    char fer_enc;       /* see FER_ENCODING */                                          /* 29 */
    char md5_here;      /* bool if non zero, the whole file MD5 is below */             /* 2A */
    char md5 [32];      /* whole file MD5 is ASCII hex */                               /* 2B */
    char md51 [32];     /* first block MD5 */                                           /* 4B */
    char reserved [21];                                                                 /* 6B */
} KWGAEncFileHeader;                                                                    /* 80 */

/* Original definition is in internal/WGA/access_countrol/src/wga/fuse/enc_reader.hpp */
static const int8_t header_table[sizeof (KWGAEncFileHeader)] =
{
    101,  -6,   -23,  5,    -93,  20,   -128, -36,      /* 00 */
    -42,  74,   -98,  104,  42,   12,   127,  37,       /* 08 */
    -47,  -61,  124,  54,   -124, -94,  47,   72,       /* 10 */
    70,   17,   -10,  108,  8,    31,   37,   -38,      /* 18 */
    104,  -6,   -117, 79,   115,  89,   33,   -93,      /* 20 */
    -47,  -105, -87,  -38,  90,   -45,  -59,  -46,      /* 28 */
    -96,  106,  15,   -87,  -110, -101, 106,  -117,     /* 30 */
    39,   73,   120,  -30,  -63,  21,   127,  -32,      /* 38 */
    98,   -104, -3,   -81,  -60,  -120, 13,   -108,     /* 40 */
    -53,  88,   123,  7,    103,  32,   -14,  -113,     /* 48 */
    -68,  -27,  44,   109,  -122, -7,   81,   -13,      /* 50 */
    64,   42,   -88,  -37,  -1,   -19,  66,   -105,     /* 58 */
    -75,  -108, -5,   -121, -86,  47,   -120, -18,      /* 60 */
    -69,  -29,  -68,  124,  -53,  -104, -28,  42,       /* 68 */
    120,  52,   -80,  -23,  -110, -101, 106,  -117,     /* 70 */
    -21,  -35,  12,   -117, 9,    -122, -21,  31        /* 78 */
};                                                      /* 80 */

static const KWGAEncFileHeader header_const =
{
    "NeCnBcIo", /* NUL terminator is significant */
    {0},{0},{0},/* not constant */
    fer_encAES, /* constant as DES and Blowfish are not implmented */
    0,{0},{0},  /* not constant */
    {0}         /* constant */
};

/* the header is obscured by XOR against a predefined pattern */
static
void KWGAEncFileHeaderDecrypt (KWGAEncFileHeader * header)
{
    size_t ix;
    int8_t * p = (int8_t*)header;

    assert (header);
    for (ix = 0; ix < sizeof (*header); ++ix)
        p[ix] ^= header_table[ix];
}

KRYPTO_EXTERN rc_t CC KFileIsWGAEnc (const void * buffer, size_t buffer_size)
{
    const uint8_t * ph;
    const uint8_t * pt;
    const uint8_t * pb;
    size_t ix;
    size_t lim;

    if ((buffer == NULL) || (buffer_size == 0))
        return RC  (rcFS, rcFile, rcIdentifying, rcParam, rcNull); 

    /* bare minimum size to identify we decide is the first 8 
     * obsfucated ASCII bytes */

    if (buffer_size < (sizeof (header_const.magic) - 1))
        return RC (rcFS, rcFile, rcIdentifying, rcBuffer, rcInsufficient); 

    /* a match is ph[X] ^ pt[X] == pb[X] at specially identified points
     * we could have put the ^ into a constant for a tad faster operation
     * but meh...
     */
    ph = (const uint8_t*)&header_const;
    pt = (const uint8_t*)&header_table;
    pb = (const uint8_t*)buffer;

    ix = offsetof (struct KWGAEncFileHeader, magic);
    lim = ix + sizeof (header_const.magic);
    if (lim > buffer_size)
        lim = buffer_size;
    for (; (ix < lim) && (ix < buffer_size); ++ix)
        if ((ph[ix] ^ pt[ix]) != pb[ix])
            return SILENT_RC (rcFS, rcFile, rcIdentifying, rcFile, rcWrongType); 

    if (buffer_size < offsetof (struct KWGAEncFileHeader, fer_enc))
        return 0;

    if ((ph[offsetof (struct KWGAEncFileHeader, fer_enc)] ^ pt[offsetof (struct KWGAEncFileHeader, fer_enc)])
        != pb[offsetof (struct KWGAEncFileHeader, fer_enc)])
        return RC (rcFS, rcFile, rcIdentifying, rcFile, rcWrongType); 


    ix = offsetof (struct KWGAEncFileHeader, reserved);
    if (buffer_size < ix)
        return 0;

    lim = ix + sizeof (header_const.reserved);

    if (lim > buffer_size)
        lim = buffer_size;
    
    for (; ix < lim; ++ix)
        if ((ph[ix] ^ pt[ix]) != pb[ix])
            return RC (rcFS, rcFile, rcIdentifying, rcFile, rcWrongType);

    return 0;
}






/* ======================================================================
 * KWGAEncFile
 */

/* -----
 * define the specific types to be used in the templatish/inheritancish
 * definition of vtables and their elements
 */
typedef struct KWGAEncFile KWGAEncFile;
#define KFILE_IMPL struct KWGAEncFile

#include <kfs/impl.h>

struct KWGAEncFile
{
    KFile dad;                  /* base class */
    const KFile * encrypted;    /* encrypted file as a read only KFile */
    uint64_t file_size;         /* as read from the encrypted file header */
    KTime_t mtime;              /* as read from the encrypted file header */
    uint32_t block_size;        /* block size is in KB  not bytes */
    KCipher * cipher;

    char inkey [32];
    size_t inkey_size;

    bool md5_here;              /* was the md5 in the header? */
    char md5[32];            /* external md5 loaded into the header as ASCII hex */
    char md51[32];           /* md5 of first block loaded into the header as ASCII hex */
    struct
    {
        uint64_t  offset;       /* position with in the unencrypted file */
        uint32_t  valid;        /* how much usable at data */
        uint8_t   data [DEFAULT_BUFF_SIZE];
    } buffer;
};


/* ----------------------------------------------------------------------
 * InitKey
 *
 * This method mimics the prepare_key method in the WGA C++ code including
 * what looks like programming errors.
 *
 * The goal is compatibility not 'correctness'.
 */
static
rc_t KWGAEncFileKeyInit (KWGAEncFile * self, const char * key, size_t key_size)
{
    rc_t rc;
    char g_key [32];  /* original used 513 but no key uses that much */

    /* scary! */
    /* this is copied directly from enc_read.cpp with all the 'possibly wrong' code */
    memset (g_key, 0, sizeof g_key);
    if (key_size > sizeof (g_key))
        key_size = sizeof (g_key);
    memmove (g_key, key, key_size);

    if (key_size < 16) /* even though g_key and some cipher keys are larger than 16 */
    {
        size_t jx, ix;
        for ((jx = key_size),(ix = 0); jx < 16; ++jx, ++ix)
        {
            g_key[jx] = self->md51[ix] | g_key[ix%(jx?jx:1)];    /* cringe? */
        }
    }

    /* okay we'll use 32 bytes from g_key though is is highly
     * likely we've only got 16 non-zero bytes */
     rc = KCipherSetDecryptKey (self->cipher, g_key, WGA_AES_BITS/8);

    return rc;
}


/* ----------------------------------------------------------------------
 * Destroy
 *
 */
static
rc_t CC KWGAEncFileDestroyRead (KWGAEncFile *self)
{
    rc_t rc = 0;

    if (self)
    {
        rc = KFileRelease (self->encrypted);
        free (self);
    }
    return rc;
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
struct KSysFile *CC KWGAEncFileGetSysFile (const KWGAEncFile *self, uint64_t *offset)
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
rc_t CC KWGAEncFileRandomAccess (const KWGAEncFile *self)
{
    assert (self != NULL);
    assert (self->encrypted != NULL);
    return KFileRandomAccess (self->encrypted);
}


/* ----------------------------------------------------------------------
 * Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t CC KWGAEncFileSize (const KWGAEncFile *self, uint64_t *size)
{
    uint64_t esize;
    rc_t rc;
    assert (self != NULL);
    assert (self->encrypted != NULL);
    assert (size != NULL);

    *size = 0;
    rc = KFileSize (self->encrypted, &esize);
    if (rc == 0)
        *size = esize - sizeof (KWGAEncFileHeader);
    return rc;
}


/* ----------------------------------------------------------------------
 * SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KWGAEncFileSetSize (KWGAEncFile *self, uint64_t size)
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

/* local fill the buffer with block or more  */
static
rc_t KWGAEncFileReadInt (KWGAEncFile * self, uint64_t pos, size_t bsize)
{
    uint64_t adjpos;
    size_t tot_read;
    size_t num_read;
    rc_t rc = 0;

    assert (self);
    assert (bsize);
    assert (bsize <= 32 * 1024);
    assert (128%16 == 0);

    memset (self->buffer.data, 0, sizeof self->buffer.data);
    tot_read = num_read = 0;
    adjpos = pos + sizeof (KWGAEncFileHeader);
#if 0
    do
    {
        rc = KFileRead (self->encrypted, adjpos + tot_read,
                        self->buffer.data + tot_read, bsize - tot_read,
                        &num_read);
        if (rc)
            return rc;        
        tot_read += num_read;
    } while ((tot_read < bsize) && (num_read > 0));
#else
    rc = KFileReadAll (self->encrypted, adjpos, self->buffer.data, bsize,
                       &tot_read);
#endif
    self->buffer.offset = pos;
    self->buffer.valid = (uint32_t)tot_read;

    if (tot_read & 15)
        rc = RC (rcKrypto, rcFile, rcReading, rcSize, rcInsufficient);
    else if (tot_read > 0)
    {
#if 1

#if RETAINED_COMPATIBILTY_WITH_ERROR
        KCipherDecryptECB (self->cipher, self->buffer.data, self->buffer.data,
                           (uint32_t)(tot_read / ECB_BYTES));
#else
/* Well this is wrong for even being wrong now */
        KCipherDecryptECB (self->cipher, self->buffer.data, self->buffer.data,
                           tot_read);
#endif

#else
        uint32_t count;
        uint32_t ix;

        /* this loop can be replaced by the KCipherDecryptECB
         * with care taken to match the error in the original
         * C++
         */

#if RETAINED_COMPATIBILTY_WITH_ERROR
        count = tot_read / ECB_BYTES;
#else
        /* do all full 16 byte blocks plus a last partial block */
        count = (tot_read + (ECB_BYTES-1)) / ECB_BYTES;
#endif
        for (ix = 0; ix < count; ++ix)
        {
            uint8_t * pb = self->buffer.data + (ix * ECB_BYTES);

            rc = KCipherDecrypt (self->cipher, pb, pb);
            if (rc)
                break;
        }
#endif
    }
    return rc;
}


static
rc_t CC KWGAEncFileRead	(const KWGAEncFile *cself,
                         uint64_t pos,
                         void *buffer,
                         size_t bsize,
                         size_t *num_read)
{
    KWGAEncFile * self = (KWGAEncFile *)cself; /* mutable values */
    rc_t rc = 0;

    assert (cself);
    assert (buffer);
    assert (num_read);


    *num_read = 0;

    /* are we even within the file? If not just say no. Drugs are bad Mmmkay */
    if (pos >= self->file_size)
    {}
    /* are we not reading from out what is already in the decrypted buffer space? */
    else
    {
        if ((self->buffer.valid == 0) ||
            (pos < self->buffer.offset) ||
            (pos >= (self->buffer.offset + self->buffer.valid)))
        {
            if (pos < self->block_size) /* we'll be reading from the first 'block' */
            {
                rc = KWGAEncFileReadInt (self, 0, self->block_size);
                if (rc)
                {
                    LOGERR (klogErr, rc, "error reading first data block of"
                            " encrypted file");
                    return rc;
                }
                if (self->buffer.valid > self->block_size)
                {
                    rc = RC (rcFS, rcFile, rcReading, rcBuffer, rcTooBig);
                    LOGERR (klogInt, rc, "read wrong amount for first block");
                    return rc;
                }
                else
                {
                    size_t md5_size;
                    size_t nudge = 0;
                    char md51_comp [32];

                    if (self->buffer.valid & (ECB_BYTES-1))
                        nudge = ECB_BYTES - (self->buffer.valid & (ECB_BYTES-1));
                    if (nudge)
                        memset (self->buffer.data + self->buffer.valid, 0, nudge);

                    md5_size = self->buffer.valid + nudge;

                    CalcMD5 (self->buffer.data, md5_size, md51_comp);

#if 1
                    if (strcase_cmp (self->md51, string_size(self->md51),
                                     md51_comp, string_size(md51_comp), 32) != 0)
#else
                    if (strncasecmp (self->md51, md51_comp, 32) != 0)
#endif
                    {
                        rc = RC (rcFS, rcFile, rcReading, rcConstraint, rcCorrupt);
                        LOGERR (klogErr, rc, "MD5 does not match in decryption");
                        return rc;
                    }
                }
            }
            else /* if (pos >= self->block_size) */
            {
                rc = KWGAEncFileReadInt (self, (pos & ~ ( uint64_t ) (16-1)),
                                         DEFAULT_BUFF_SIZE);
                if (rc)
                {
                    LOGERR (klogErr, rc, "error reading data block of"
                            " encrypted file");
                    return rc;
                }
            }  /* if (pos < self->block_size) */
        } /* if ((self->buffer.valid == 0) || etc. */
        /* if here we have a valid buffer and it contains the start pos requested */
/*     assert (pos >= self->buffer.offset); */
/*     assert (pos < (self->buffer.offset +self->buffer.valid)); */
        {
            size_t start;
            size_t limit;

            /* find offset of start for copy within the buffer */
            start = (size_t)(pos - self->buffer.offset);
            /* how many bytes available starting here */
            limit = self->buffer.valid - start;

            if (pos + limit > self->file_size)
                limit = self->file_size - pos;

            /* are we asking to read more than we have? is so trim the request */
            if (limit < bsize)
                bsize = limit;

            memmove (buffer, self->buffer.data + start, bsize);
            *num_read = bsize;
        }
    }
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
rc_t CC KWGAEncFileWriteFail (KWGAEncFile *self, uint64_t pos,
                              const void *buffer, size_t bsize,
                              size_t *num_writ)
{
    assert (self);
    return RC (rcFS, rcFile, rcReading, rcFunction, rcUnsupported);
}


/* ----------------------------------------------------------------------
 * Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KWGAEncFileType (const KWGAEncFile *self)
{
    assert (self != NULL);
    assert (self->encrypted != NULL);

    return KFileType (self->encrypted);
}


static const KFile_vt_v1 vtKWGAEncFileRead =
{
    /* version */
    1, 1,

    /* 1.0 */
    KWGAEncFileDestroyRead,
    KWGAEncFileGetSysFile,
    KWGAEncFileRandomAccess,
    KWGAEncFileSize,
    KWGAEncFileSetSize,
    KWGAEncFileRead,
    KWGAEncFileWriteFail,

    /* 1.1 */
    KWGAEncFileType
};


static
rc_t KWGAEncFileHeaderRead (KWGAEncFile * self)
{
    KWGAEncFileHeader header;
    uint8_t * pb;
    size_t num_read;
    size_t tot_read;
    rc_t rc;

    assert (self);
    assert (sizeof (KWGAEncFileHeader) == 128);


    DEBUG_STS (("s: Enter '%p'\n", __func__, self));
    pb = (void*)&header;
    for (num_read = tot_read = 0; tot_read < sizeof header; )
    {
        rc = KFileRead (self->encrypted, (uint64_t)tot_read, pb, 
                        sizeof (header) - tot_read, &num_read);
        if (rc)
        {
            LOGERR (klogErr, rc, "Error reading the header for an encrypted file");
            return rc;
        }

        if (num_read == 0)
        {
            rc =  RC (rcFS, rcFile, rcReading, rcFile, rcInsufficient);
            LOGERR (klogErr, rc, "Header incomplete for an encrypted file");
            return rc;
        }
        tot_read += num_read;
        pb += num_read;
    }

    KWGAEncFileHeaderDecrypt (&header);

    if (memcmp (header.magic, ncbi_crypt_magic, sizeof ncbi_crypt_magic) != 0)
    {
        rc = RC (rcFS, rcFile, rcReading, rcHeader, rcCorrupt);
        LOGERR (klogErr, rc, "Header's magic bad for encrypted file");
        return rc;
    }

    /* so far unknown legal range */
    self->block_size = strtou32 (header.block_sz, NULL, KWGA_ENC_FILE_HEADER_RADIX);

    self->file_size = strtou64 (header.file_sz, NULL, KWGA_ENC_FILE_HEADER_RADIX);

    /* file format has limiting feature of a 32 bit timestamp */
    self->mtime = (KTime_t)strtol (header.mtime, NULL, KWGA_ENC_FILE_HEADER_RADIX);

    switch ((FER_ENCODING)header.fer_enc)
    {
    default:
        rc = RC (rcFS, rcFile, rcReading, rcHeader, rcOutofrange);
        LOGERR (klogErr, rc, "Enryption type code out of range");
        return rc;
    case fer_encDES:
    case fer_encBLF:
        rc = RC (rcFS, rcFile, rcReading, rcHeader, rcIncorrect);
        LOGERR (klogErr, rc, "Enryption type code not supported");
        return rc;
    case fer_encAES:
        break;
    }

    self->md5_here = (header.md5_here != 0);

    if (self->md5_here)
        memmove (self->md5, header.md5, sizeof (self->md5));

    memmove (self->md51, header.md51, sizeof (self->md51));

    return 0; /* yeah not really checking later errors am i? */
}

/* ----------------------------------------------------------------------
 * KWGAEncFileMake
 *  create a new file object
 *
 * pself        where to put pointer to new object
 * encrypted    a readable file that is encrypted
 * cipher       a cipher object that must support AES with 256 bit key
 *              not verified except through failure to decrypt a file
 * key          bytes containing the user key - should be utf-8 but with no
 *              control characters
 * key_size     how many bytes to use from the key 
 */
/* read only version for decrypting of existing files */
KRYPTO_EXTERN rc_t CC KFileMakeWGAEncRead (const struct KFile ** pself, 
                                           const struct KFile * encrypted,
                                           const char * key,
                                           size_t key_size)
{
    rc_t rc;

    if (pself == NULL)
    {
        rc = RC (rcFS, rcFile, rcConstructing, rcSelf, rcNull);
        LOGERR (klogErr, rc, "key parameter for WGA encrypted file is empty");
        return rc;
    }

    *pself = NULL;

    if ((encrypted == NULL)||(key == NULL))
    {
        rc = RC (rcFS, rcFile, rcConstructing, rcParam, rcNull);
        LOGERR (klogErr, rc, "missing WGA encrypted file passed in to constructor");
    }

    else if (key_size == 0)
    {
        rc = RC (rcFS, rcFile, rcConstructing, rcParam, rcInvalid);
        LOGERR (klogErr, rc, "missing WGA encrypted file passed in to constructor");
    }

    else if (encrypted->read_enabled == 0)
    {
        rc = RC (rcFS, rcFile, rcConstructing, rcParam, rcIncorrect);
        LOGERR (klogErr, rc, "encrypted file not readable");
    }
    else
    {
        KCipherManager * cipher_mgr;

        rc = KCipherManagerMake (&cipher_mgr);
        if (rc == 0)
        {
            KCipher * cipher;

            rc = KCipherManagerMakeCipher (cipher_mgr, &cipher, kcipher_AES);
            if (rc == 0)
            {
                size_t z;

                rc = KCipherBlockSize (cipher, &z);
                if (rc)
                {
                    LOGERR (klogErr, rc, "unable to get block size for WGA "
                            "encrypted file cipher passed in to constructor");
                }
                else
                {
                    if (z != ECB_BYTES)
                    {
                        rc = RC (rcFS, rcFile, rcConstructing, rcParam,
                                 rcInvalid);
                        LOGERR (klogErr, rc, "wrong block size for WGA "
                                "encrypted file cipher passed in to "
                                "constructor");
                    }
                    else
                    {
                        KWGAEncFile * self;

                        self = calloc (sizeof (*self), sizeof (uint8_t));
                        if (self == NULL)
                        {
                            rc = RC (rcFS, rcFile, rcConstructing, rcMemory,
                                     rcExhausted);
                            LOGERR (klogErr, rc, "out of memory while "
                                    "constructing decryptor");
                        }
                        else
                        {
                            rc = KFileAddRef (encrypted);
                            if (rc)
                                LOGERR (klogErr, rc, "unable to add reference "
                                "to encrypted file");
                            else
                            {
                                /* cast to strip const */
                                self->encrypted = encrypted;
                                self->cipher = cipher;

                                /* read the header of the encrypted file for
                                 * details about the  decrypted file */
                                DEBUG_STS(("%s: calling KWGAEncFileHeaderRead\n",
                                           __func__));
                                rc = KWGAEncFileHeaderRead (self);
                                if (rc == 0)
                                {
                                    /* using the file header's stored encoding
                                     * key build a key from the parameter key */
                                    DEBUG_STS(("%s: calling "
                                               "KWGAEncFileHeaderRead\n",
                                               __func__));
                                    rc = KWGAEncFileKeyInit (self, key,
                                                             key_size);
                                }
                                if (rc == 0)
                                {
                                    rc = KFileInit (&self->dad, 
                                                    (const KFile_vt*)
                                                    &vtKWGAEncFileRead,
                                                    "KWGAEncFile", "no-name",
                                                    true, false);
                                    if (rc)
                                        LOGERR (klogInt, rc, "Failed to initialize decrypting file");
                                    else
                                    {
                                        *pself = &self->dad;
                                        self->buffer.offset = 0;
                                        self->buffer.valid = 0;
                                        KCipherManagerRelease (cipher_mgr);
                                        return 0;
                                    }
                                }
                                /* release of encrypted handled in destroy() */
                            }
                            KWGAEncFileDestroyRead (self);
                        }
                    }
                }
                KCipherRelease (cipher);
            }           
        }
    }
    return rc; 
}

static __inline__
rc_t WGAEncValidateHeader (const KWGAEncFileHeader * header,
                           size_t header_size)
{
    rc_t rc = 0;
    unsigned ix;

    if (header_size != sizeof * header)
    {
        rc = RC (rcKrypto, rcFile, rcValidating, rcHeader, rcInsufficient);
        LOGERR (klogErr, rc, "incomplete header");
        return rc;
    }

    if (memcmp (header, &header_const, sizeof header->magic) != 0)
    {
        rc = RC (rcKrypto, rcFile, rcValidating, rcHeader, rcInvalid);
        LOGERR (klogErr, rc, "bad signature in encrypted file header");
        return rc;
    }

#if 0
    /* check block_size */
    do
    {
        for (ix = 0; isspace(header->block_sz[ix]); ++ix)
            ;

        if (ix >= sizeof (header->block_sz))
            break;

        if (header->block_sz[ix] == '+')
            ++ix;

        if (ix >= sizeof (header->block_sz))
            break;

        for (; (ix < sizeof header->block_sz); ++ix)
        {
            if (isdigit (header->block_sz[ix]))
                ;
            else if ((header->block_sz[ix] >= 'a') &&
                     (header->block_sz[ix] <= 'w'))
                ;
            else if ((header->block_sz[ix] >= 'A') &&
                     (header->block_sz[ix] <= 'W'))
                ;
            else
                break;
        }
        for (; (ix < sizeof header->block_sz); ++ix)
        {
            if (header->block_sz[ix] == '\0')
                ;
            else
                break;
        }
        if (rc)
        


        for (ix = 0; ix < sizeof header->block_size; ++ix)
        {
            if ()
                ;
                }
        
            ;
        else if (header->block_sz[ix] == '-')
            ;
        else
            ;
    }while (0);
    /* check file_sz */
    /* check mtime */
#endif

    if (header->fer_enc != fer_encAES)
    {
        rc = RC (rcKrypto, rcFile, rcValidating, rcHeader, rcInvalid);
        LOGERR (klogErr, rc, "bad encoding flag in encrypted file header");
        return rc;
    }

    if ((header->md5_here != true) && (header->md5_here != false))
    {
        rc = RC (rcKrypto, rcFile, rcValidating, rcHeader, rcInvalid);
        LOGERR (klogErr, rc, "bad checksum flag in encrypted file header");
        return rc;
    }

    /* check md5 */
    /* check md51 */

    for (ix = 0; ix < sizeof header->reserved; ++ix)
    {
        if (header->reserved[ix] != 0)
        {
            rc = RC (rcKrypto, rcFile, rcValidating, rcHeader, rcInvalid);
            LOGERR (klogErr, rc, "bad reserved space in encrypted file header");
            return rc;
        }
    }

    return 0;
}


KRYPTO_EXTERN rc_t CC WGAEncValidate (const KFile * encrypted, 
                                      const char * key,
                                      size_t key_size)
{
    const KFile * buffile;
    rc_t rc, orc;

    rc = KBufFileMakeRead (&buffile, encrypted, 64*1024);
    if (rc)
        LOGERR (klogErr, rc, "unable to buffer encrypted file");

    else
    {
        KWGAEncFileHeader header;
        size_t num_read;

        /* first let's disect the header */
        rc = KFileReadAll (buffile, 0, &header, sizeof header, &num_read);
        if (rc)
            LOGERR (klogErr, rc, "unable to read encrypted file header");

        else
        {
            uint64_t file_size;
            uint64_t header_file_size;
            char ascii_md5 [32];
            char header_ascii_md5 [32];

            KWGAEncFileHeaderDecrypt (&header);

            rc = WGAEncValidateHeader (&header, num_read);
            if (rc)
                ;

            else
            {
                if (key_size == 0)
                {
                    const KFile * countfile;
                    rc = KFileMakeCounterRead (&countfile, encrypted, &file_size, NULL, false);
                    if (rc)
                    {
                        LOGERR (klogErr, rc, "error making file size counter");
                        file_size = 0;
                    }
                    else
                    {
                        KFileAddRef (encrypted);
                        KFileRelease (countfile);

                        file_size -= sizeof header;
                    }
                }
                else
                {
                    union
                    {
                        const KFile * decrypted;
                        const KWGAEncFile * decryptor;
                    } u;

                    rc = KFileMakeWGAEncRead (&u.decrypted, buffile, key, key_size);
                    if (rc)
                        LOGERR (klogErr, rc, "error making decryptor");

                    else
                    {
                        KFile * nullfile;

                        rc = KFileMakeNullUpdate (&nullfile);
                        if (rc)
                            LOGERR (klogInt, rc, "error making data sync");

                        else
                        {
                            KMD5SumFmt * fmt;
                            static const char name[] = "wgaencrypt";

                            rc = KMD5SumFmtMakeUpdate (&fmt, nullfile);
                            if (rc)
                            {
                                LOGERR (klogInt, rc, "error making md5sum database");
                                KFileRelease (nullfile);
                            }
                            else
                            {
                                const KFile * md5file;

                                rc = KFileMakeNewMD5Read (&md5file, u.decrypted, fmt, name);
                                if (rc)
                                    LOGERR (klogInt, rc, "error making MD5 calculator");

                                else
                                {
                                    /*
                                     * ill mannered md5file steals
                                     * references instead of adding one
                                     */
                                    rc = KFileAddRef (u.decrypted);
                                    if (rc)
                                        LOGERR (klogInt, rc, "error adding reference to decryptor");

                                    else
                                    {

                                        const KFile * countfile;
                                        rc = KFileMakeCounterRead (&countfile,
                                                                   md5file,
                                                                   &file_size, NULL,
                                                                   true);
                                        if (rc)
                                            LOGERR (klogErr, rc,
                                                    "error making file size "
                                                    "counter");
                                        else
                                        {
                                            /*
                                             * ill mannered countfile steals
                                             * references instead of adding one
                                             */
                                            KFileAddRef (md5file);
                                            rc = KFileRelease (countfile);
                                        }

                                        orc = KFileRelease (md5file);
                                    
                                        if (rc == 0)
                                            rc = orc;

                                        if (rc == 0)
                                        {
                                            uint8_t md5 [16];
                                            bool bin;

                                            rc = KMD5SumFmtFind (fmt, name, md5, &bin);
                                            if (rc)
                                                LOGERR (klogInt, rc, "error locating MD5");

                                            else
                                            {
                                                size_t zz;
                                                unsigned ix;
                                                for (ix = 0; ix < 16; ++ix)
                                                {
                                                    string_printf (&ascii_md5[2*ix], 2, &zz, "%2.2x",
                                                               (unsigned)(uint8_t)md5[ix]);
                                                }
                                                memmove (header_ascii_md5,
                                                        u.decryptor->md5,
                                                        sizeof header_ascii_md5);
                                            }
                                        }
                                    }
                                }
                                KMD5SumFmtRelease (fmt);
                            }
                        }
                        KFileRelease (u.decrypted);
                    }                
                }
            }
            if (rc == 0)
            {
                uint64_t sys_file_size;
                uint64_t pad_file_size = 0;
                rc_t orc;

                /* ccheck file size */

                orc = KFileSize (encrypted, &sys_file_size);
                if (orc == 0)
                {
                    pad_file_size = file_size + 15;
                    pad_file_size &= ~ ( uint64_t ) 15;
                    pad_file_size += sizeof (KWGAEncFileHeader);
                }
                header_file_size = strtou64 (header.file_sz, NULL, KWGA_ENC_FILE_HEADER_RADIX);

                if (key_size == 0)
                {
                    header_file_size += 15;
                    header_file_size &= ~ ( uint64_t ) 15;
                }

                if (file_size < header_file_size)
                    rc = RC (rcKrypto, rcFile, rcValidating, rcSize, rcInsufficient);
                
                else if (file_size > header_file_size)
                    rc = RC (rcKrypto, rcFile, rcValidating, rcSize, rcExcessive);

                else if ((orc == 0) &&
                         (sys_file_size > pad_file_size))
                    rc = RC (rcKrypto, rcFile, rcValidating, rcSize, rcExcessive);

                /* check md5 */
                else if (!header.md5_here)
                {
                    /* rc = RC (rcKrypto, rcFile, rcValidating, rcEncryption, rcNotFound); */
                }
                else if (key_size == 0)
                    rc = RC (rcKrypto, rcFile, rcValidating, rcEncryption, rcNull);

                else
                {
                    int cmp;

                    cmp = strcase_cmp (ascii_md5, sizeof ascii_md5, 
                                       header_ascii_md5, sizeof header_ascii_md5,
                                       sizeof ascii_md5);
                    if (cmp)
                    {
                        rc = RC (rcKrypto, rcFile, rcValidating, rcChecksum,
                                 rcInvalid);
                        LOGERR (klogErr, rc, "Encrypted file MD5 does not match");
                    }
                }
            }
        }
        KFileRelease (buffile);
    }
    return rc;
}
#if 0
        /* not working as planned */
        if (rc == 0)
        {
            uint32_t this_read;

            /* try to read past size now obtained */
            do
            {
                uint8_t throwaway [8192];

                rc = KFileRead (encrypted, z + sizeof (KWGAEncFileHeader), throwaway, sizeof throwaway,
                                &this_read);
                if (rc)
                {
                    LOGERR (klogErr, rc, "Error reading file");
                    break;
                }
                z += this_read;

            } while (this_read);
        }
#endif

/* end of file wgaencrypt.c */

