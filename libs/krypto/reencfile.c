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
#include <krypto/reencfile.h>
#include <krypto/key.h>
#include <krypto/encfile.h>
#include <krypto/encfile-priv.h>
#include "encfile-priv.h"

#include <klib/rc.h>
#include <klib/checksum.h>
#include <klib/log.h>
#include <klib/out.h>
#include <klib/debug.h>
#include <klib/vector.h>
#include <klib/status.h>
#include <kfs/file.h>
#include <kfs/ramfile.h>

#include <sysalloc.h>

#include <byteswap.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <klib/out.h>

#define USE_MISSING_VECTOR false

/* ----------------------------------------------------------------------
 * KReencFile
 *   Base object class for the encryption, decryption and validation of
 *   the file format defined above
 */
#define KFILE_IMPL KReencFile
#include <kfs/impl.h>

/* -----
 */
struct KReencFile
{
    KFile         dad;          /* base class */
    const KFile * encrypted;    /* encrypted file we start from */
    const KFile * dec;          /* decryptor of the original */
    KFile *       enc;          /* encryptor */
    KFile *       ram;          /* a file that works from a sliding window buffer */

    size_t        num_read;
    size_t        num_writ;
/* block id's can not max out a 64 bit number as that is a file that is 32K times too big */
#define NO_CURRENT_BLOCK (~(uint64_t)0)
    uint64_t      block_id;
    uint64_t      footer_block; /* if zero, file does not have any content blocks */

    uint64_t      size;         /* size as known from the original file [see known_size] */
    bool          known_size;   /* is the size of the original file known? */

    char          plain_text  [ENC_DATA_BLOCK_SIZE];
    union
    {
        KEncFileBlock block;
        char          text [sizeof (KEncFileBlock)];
    } block;
    union
    {
        KEncFileFooter foot;
        char           text [sizeof (KEncFileFooter)];
    } foot;

};

/* ----------------------------------------------------------------------
 * Interface Functions
 *
 * Destroy
 *
 */
static
rc_t CC KReencFileDestroy (KReencFile *self)
{
    if (self)
    {
        rc_t rc1, rc2, rc3, rc4;

        rc1 = KFileRelease (self->encrypted);
        if (rc1)
            LOGERR (klogInt, rc1, "Re-enc failed to release encrypted file");

        rc2 = KFileRelease (self->dec);
        if (rc2)
            LOGERR (klogInt, rc2, "Re-enc failed to release decryptor");

        rc3 = KFileRelease (self->ram);
        if (rc3)
            LOGERR (klogInt, rc3, "Re-enc failed to release ram file");

        rc4 = KFileRelease (self->enc);
        if (rc4)
            LOGERR (klogInt, rc4, "Re-enc failed to release encryptor");

        free (self);

        return (rc1 ? rc1 :
                rc2 ? rc2 :
                rc3 ? rc3 :
                rc4);
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
struct KSysFile *CC KReencFileGetSysFileUnsupported (const KReencFile *self, uint64_t *offset)
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
rc_t CC KReencFileRandomAccess (const KReencFile *self)
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
rc_t CC KReencFileSize (const KReencFile *self, uint64_t *size)
{
    assert (self != NULL);
    assert (self->encrypted != NULL);

    /* -----
     * the re-encrypted file will be the same size as the 
     * previously encrypted file and we have the same understanding
     * about knowing the size
     */
    return KFileSize (self->encrypted, size);
}


static
rc_t CC KEncryptFileSize (const KReencFile *self, uint64_t *size)
{
    uint64_t z;
    rc_t rc;

    assert (self != NULL);
    assert (self->encrypted != NULL);

    /* -----
     * the re-encrypted file will be the same size as the 
     * previously encrypted file and we have the same understanding
     * about knowing the size
     */
    rc = KFileSize (self->encrypted, &z);

    if (rc == 0)
    {
        uint64_t bid = PlaintextSize_to_BlockCount ( z, NULL );
        *size = BlockId_to_CiphertextOffset ( bid ) + sizeof ( KEncFileFooter );
    }
    else
    {
        *size = z;
    }
    return rc;
}


/* ----------------------------------------------------------------------
 * SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KReencFileSetSizeUnsupported (KReencFile *self, uint64_t size)
{
    assert (self);

    return RC ( rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported );
}


/*
 * The next three functions do the actual Out from a KFileRead.
 * We will only out from the header, or a footer or a single
 * encrypted block rather than try to fully satisfy all of a
 * KFileRead. USe KFileReadAll to get more than one part
 *
 * For all we have an offset within the part which is the
 * position from the original read request for the header
 */
static __inline__
rc_t KReencFileReadHeaderOut (KReencFile * self, size_t offset, void * buffer,
                              size_t bsize, size_t *num_read)
{
    assert (self);
    assert (offset < sizeof (KEncFileHeader));
    assert (buffer);
    assert (bsize);
    assert (num_read);

    /* trim request if necessary */
    if (offset + bsize > sizeof (KEncFileHeader))
        bsize = sizeof (KEncFileHeader) - offset;

    memmove (buffer, self->block.text + offset, bsize);
    self->block_id = NO_CURRENT_BLOCK;
    *num_read = bsize;
    
    return 0;
}


static __inline__
rc_t KReencFileReadBlockOut (KReencFile * self, size_t offset, void * buffer,
                             size_t bsize, size_t * num_read)
{
    assert (self);
    assert (offset < sizeof self->block);
    assert (buffer);
    assert (bsize);
    assert (num_read);

    if (offset + bsize > sizeof self->block)
        bsize = sizeof self->block - offset;

    memmove (buffer, self->block.text + offset, bsize);
    *num_read = bsize;

    return 0;
}


static __inline__
rc_t KReencFileReadFooterOut (KReencFile * self, size_t offset,
                              void * buffer, size_t bsize, size_t * num_read)
{
    /* for the footer we will copy out from the footer in self */

    assert (self);
    assert (offset < sizeof self->foot);
    assert (buffer);
    assert (bsize);
    assert (num_read);

/* KOutMsg ("%s: offset '%zu' bsize '%zu'",__func__,offset,bsize); */
    if (offset + bsize > sizeof self->foot)
        bsize = sizeof self->foot - offset;

    memmove (buffer, self->foot.text + offset, bsize);
    self->block_id = NO_CURRENT_BLOCK;
    *num_read = bsize;

/* KOutMsg (" *num_read '%zu'\n",bsize); */
/* { */
/*     size_t ix; */
/*     char * b = buffer; */
/*     KOutMsg ("%s:",__func__); */
/*     for (ix = 0; ix < bsize; ++ix) */
/*         KOutMsg (" %2.2x",b[ix]); */
/*     KOutMsg ("\n"); */

/* } */
    return 0;
}


/*
 * Handle Read within the Encrypted file header
 *
 * We use a private interface into the KEncFile then 
 * the Out function below to take what we write with
 * the Encryptor and copy it to the callers read buffer.
 */
static
rc_t KReencFileReadHandleBlock (KReencFile *self,
                                uint64_t pos,
                                void *buffer,
                                size_t bsize,
                                size_t *num_read);

static __inline__
rc_t KReencFileReadHandleHeader (KReencFile *self,
                                 uint64_t pos,
                                 void *buffer,
                                 size_t bsize,
                                 size_t *num_read)
{
    rc_t rc;

    assert (self);
    assert (pos < sizeof (KEncFileHeader));
    assert (buffer);
    assert (bsize);
    assert (num_read);

    if ( !self->known_size || self->size != sizeof(KEncFileHeader) + sizeof(KEncFileFooter) )
    {
        /* added to support NCBInenc and NCBIsenc variants of KEncFile
         * read the first block of the source file but don't write any of it
         *
         * If the source is stream mode only then the next block to be read is the
         * first and this will not violate by seeking. It will just have already
         * read the block it will want to read real soon. Likewith within a
         * single call to KFileReadAll.
         */
        rc = KReencFileReadHandleBlock (self, 0, NULL, 0, NULL);
        if (rc)
            return rc;
    }

    /* use a private function from KEncFile to generate a header */
    rc = KEncFileWriteHeader (self->enc);
    if (rc)
        LOGERR (klogErr, rc, "re-enc error generating encrypted header");
    else
    {
        /*
         * assume it worked and its the right size 
         * copy the requested portion of the header out to finish the read
         */
        rc = KReencFileReadHeaderOut (self, pos, buffer, bsize, num_read);
        if (rc)
            LOGERR (klogErr, rc, "re-enc error filling read request");
    }
    return rc;
}


/*
 * Read the requested block form the source encrypted file
 */
static
rc_t KReencFileReadABlock (KReencFile * self, uint64_t block_id)
{
    rc_t rc;


/*     OUTMSG (("%s: block_id %lu\n",__func__,block_id)); */

    if (block_id + 1 == self->footer_block)
        memset (self->plain_text, 0, sizeof self->plain_text);

    /* -----
     * simple call down to the decryptor to get the plain text data
     * for this block. We will regenerate the framing when we re-encrypt
     */
    rc = KFileReadAll (self->dec, BlockId_to_PlaintextOffset ( block_id ),
                       &self->plain_text, sizeof (self->plain_text),
                       &self->num_read);
    if (rc)
        LOGERR (klogErr, rc, "re-enc error reading a block");

    /*
     * interpret bytes read for validity
     *
     * zero means we hit a premature end of file where we expected a block
     */
    else if (self->num_read == 0)
    {
        /*misleading RC? */
        rc = RC (rcKrypto, rcFile, rcReading, rcSize, rcInsufficient);
        LOGERR (klogErr, rc, "re-enc no block read");
    }
    /*
     * less than a full block must be in the last block
     */
    else if (self->num_read < sizeof self->plain_text)
    {
/*         KOutMsg ("%s: block_id '%lu'num_read '%zu' of '%zu' last_block '%lu'\n", */
/*                  __func__, block_id, self->num_read, sizeof self->plain_text, */
/*                  self->footer_block); */

        if (block_id + 1 != self->footer_block)
        {
            rc = RC (rcKrypto, rcFile, rcReading, rcSize, rcInsufficient);
            LOGERR (klogErr, rc, "re-enc incomplete block read");
        }
    }
    /*
     * unlikely scenario, read too much
     */
    else if (self->num_read > sizeof self->plain_text)
    {
        rc = RC (rcKrypto, rcFile, rcReading, rcBuffer, rcIncorrect);
        LOGERR (klogErr, rc, "re-enc no block read");
    }
    if (rc == 0)
        self->block_id = block_id;
    return rc;
}


static
rc_t KReencFileWriteABlock (KReencFile * self, uint64_t block_id)
{
    rc_t rc;

    /* -----
     * simple call to encrypt an entire data section for the relevant block
     * We send in up to 32KB of plain text which through two element KFiles will
     * we written into a buffer back in this KFile. More data will be written
     * to that buffer than requested here - that is the framing and also the
     * header if the block is the first one.
     */
    rc = KFileWriteAll (self->enc, BlockId_to_PlaintextOffset ( block_id ),
                         self->plain_text, self->num_read, &self->num_writ);

    if (rc)
        LOGERR (klogInt, rc, "re-enc error encrypting a block");

    else if (self->num_writ != self->num_read)
    {
        rc = RC (rcKrypto, rcFile, rcWriting, rcFile, rcIncomplete);
        LOGERR (klogErr, rc, "re-enc failure encrypting all of block");
    }

    /* trigger a flush */
/*     if (rc == 0) */
/*         rc = KFileWriteAll (self->enc, BlockId_to_DecryptedPos (block_id^1), */
/*                          self->plain_text, self->num_read, &self->num_writ); */

    return rc;
}


/*
 * Add the current encrypted block to the footer statistics
 */
static __inline__
rc_t KReencFileAddToFooter (KReencFile * self)
{
    assert (self);
/*     KOutMsg ("%s: ",__func__);  */

    if (self->block.block.crc != self->block.block.crc_copy)
    {
        rc_t rc = RC (rcKrypto, rcFile, rcReading, rcChecksum, rcInvalid);
        LOGERR (klogInt, rc, "rc-enc block CRCs disagree");
        return rc;
    }

    ++ self->foot.foot.block_count;
    self->foot.foot.crc_checksum += self->block.block.crc;

/*     KOutMsg ("%lu %lu %lu\n",self->foot.foot.block_count,self->block.block.crc,self->foot.foot.crc_checksum); */
    return 0;
}


/*
 * Read a block from the source encrypted block and Write it which reencrypts it
 *
 * The new_block parameter says whether this is the first time we've seen this
 * block. If it is we need to add data to the footer
 */
static
rc_t KReencFileReencBlock (KReencFile * self, uint64_t block_id, bool new_block)
{
    rc_t rc;

    assert (self);

/*     KOutMsg ("%s: %lu %lu\n", __func__, block_id, self->footer_block); */
    assert (block_id <= self->footer_block);
    assert ((new_block == true) || (new_block == false));

    rc = KReencFileReadABlock (self, block_id);
    if (rc)
        LOGERR (klogErr, rc, "re-enc failure to read a block");
    else
    {
        if ((self->num_read == 0) || (self->num_read > sizeof (self->plain_text)))
        {
            rc = RC (rcFS, rcFile, rcReading, rcSize, rcIncorrect);
            LOGERR (klogErr, rc, "Bad length on block read of encrypted file");
        }
        else
        {
            rc = KReencFileWriteABlock (self, block_id);
            if (rc)
                LOGERR (klogErr, rc, "re-enc failure to write a block");

            else if (new_block)
            {
                rc = KReencFileAddToFooter (self);
                if (rc)
                    LOGERR (klogErr, rc,
                            "re-enc failure to do block accounting");
            }
        }
    }
    return rc;
}


/*
 * Handle Read within the Encrypted file footer
 */
static __inline__
rc_t KReencFileReadHandleFooter (KReencFile *self,
                                 uint64_t pos,
                                 void *buffer,
                                 size_t bsize,
                                 size_t *num_read)
{
    uint64_t block_id;  /* block id for the footer gets us the start of footer */
    size_t offset;
    rc_t rc;

/* KOutMsg ("\n\n\n%s: pos '%lu' bsize '%zu'\n",__func__,pos,bsize); */

    assert (self);
    assert (pos >= sizeof (KEncFileHeader));
    assert (buffer);
    assert (bsize);
    assert (num_read);

    rc = 0;
    block_id = EncryptedPos_to_BlockId (pos, NULL, NULL);

    assert (block_id == self->footer_block);

    offset = pos - BlockId_to_CiphertextOffset ( block_id );

    assert (offset < sizeof self->foot);

    /* if we are tying to treat this as a footer but it wasn't the next
     * expected block mark all inbetween as missing to handle in the 
     * function just below this
     */

/* KOutMsg ("%s: self->next_block %lu\n",__func__, self->next_block); */

    self->foot.foot.block_count = block_id;
    self->foot.foot.crc_checksum = 0;

    if (rc == 0)
    {
        uint64_t header_pos;

        header_pos = BlockId_to_CiphertextOffset ( block_id );

        assert (header_pos <= pos);
        assert (pos - header_pos <= sizeof self->foot);

        rc = KReencFileReadFooterOut (self, (size_t)(pos - header_pos),
                                      buffer, bsize, num_read);

/*             KOutMsg ("%s: footer '%lu' '%lx'\n",__func__, */
/*                      self->foot.foot.block_count, */
/*                      self->foot.foot.crc_checksum); */

        if (rc)
            LOGERR (klogInt, rc, "re-enc failed to output footer");
    }
    return rc;
}


/*
 *
 */
static
rc_t KReencFileReadHandleBlock (KReencFile *self,
                                uint64_t pos,
                                void *buffer,
                                size_t bsize,
                                size_t *num_read)
{ 
    rc_t rc = 0;            /* we have a path where we need to check this without set */
    uint64_t block_id;      /* block id for the requeted position */
    uint32_t offset;        /* how far into the encrypted block */
    bool new_block;         /* is this the first time for this block */

    /* -----
     * figure out what block this corresponds to. 
     * the header is for this purpose part of the first block
     * when we decide what to give the reader. We only care
     * about which block and not whether it is in the payload
     * or framing.
     * This block id is not to a known to exist block. It could be 
     * to the header, the footer or past the end of the file.
     *
     * NOTE: This could be a pre-fetch of the first block when
     * processing the header - it will have some funny values
     * with just a self and all other parameters 0s.
     * pos of zero gives block_id of 0 and the bsize of zero
     * means we never look at the others. Thi sis to allow the 
     * (at the time this was written) new feature of two
     * different file-signatures for encrypted files.
     */
    block_id = EncryptedPos_to_BlockId (pos, NULL, NULL);

    if (block_id != self->block_id)
    {
        new_block = true;

        if (rc == 0)
        {
            /* read requested block */
            rc = KReencFileReencBlock (self, block_id, new_block);
            if (rc)
            {
                LOGERR (klogErr, rc,
                        "re-enc failure re-encryptinng a requested block");
            }
        }
    }
    if ((rc == 0) && (bsize > 0))
    {
        /* satisfy read request
         *
         * if we are here we decrypted and re-encrypted the
         * expected block
         */
        offset = ( uint32_t ) ( pos - BlockId_to_CiphertextOffset ( block_id ) );
        rc = KReencFileReadBlockOut (self, offset, buffer, bsize, num_read);
        if (rc)
            LOGERR (klogErr, rc, "re-enc error copying out from block");
    }
    return rc;
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
#if 0
static
rc_t CC KReencFileReadUnsupported (const KReencFile *self,
                                   uint64_t pos,
                                   void *buffer,
                                   size_t bsize,
                                   size_t *num_read)
{
    assert (self);
    assert (buffer);
    assert (bsize);
    assert (num_read);

    return RC ( rcFS, rcFile, rcReading, rcFunction, rcUnsupported );
}
#endif

/*
 * Read will often return only a partial read.
 *
 * We take the less complex route here and return only from the header, a
 * single block or the footer, whatever is at the beginning of the requested
 * region.
 */
static
rc_t CC KReencFileRead (const KReencFile *self_,
                        uint64_t pos,
                        void *buffer,
                        size_t bsize,
                        size_t *num_read)
{
    /* make it all mutable and stop using self_ */
    KReencFile * self = (KReencFile *)self_;
    rc_t rc = 0;

    assert (self);
    assert (buffer);
    assert (bsize);
    assert (num_read);

    *num_read = 0;

    /* -----
     * the size of the re-encrypted file will be the same as the size of the
     * previously encrypted file so we can bail early if we know we can.
     *
     * The initial use case is that the file will just be lying out on disk
     * and a KSysfile so that we do know the size.
     *
     * There are three pieces to an encrypted file:
     * 1 Header
     * 2 One or More Blocks
     * 3 Footer
     *
     * We will only know if we are in the footer if we already know the size of
     * the file. Else we have to find the footer by not finding a block when we
     * try to read it. We'll thus look for the header first, then the footer else
     * we try to find a block,
     */

    /* Header */
    if (pos < sizeof (KEncFileHeader))
        rc = KReencFileReadHandleHeader (self, pos, buffer, bsize, num_read);

    /* if past the whole encrypted file */
    else if (pos >= self->size)
        rc = 0;

    /* Footer */
    else if (pos >= self->size - sizeof self->foot)
        rc = KReencFileReadHandleFooter (self, pos, buffer, bsize, num_read);

    /* Blocks */
    else
        rc = KReencFileReadHandleBlock (self, pos, buffer, bsize, num_read);

    return rc;
}


/* ----------------------------------------------------------------------
 * Write
 *  write file at known position
 *
 *  "pos" [ IN ] - starting position within file
 *  "buffer" [ IN ] and "size" [ IN ] - data to be written
 *
 *  "num_writ" [ OUT, NULL OKAY ] - optional return parameter
 *  giving number of bytes actually written
 *
 * Unsupported as we now treat archives as READ ONLY
 */
static
rc_t CC KReencFileWriteUnsupported (KReencFile *self, uint64_t pos,
                                    const void *buffer, size_t bsize,
                                    size_t *num_writ)
{
    rc_t rc = RC (rcFS, rcFile, rcReading, rcFunction, rcUnsupported);

    assert (self);
    assert (buffer);
    assert (bsize);
    assert (num_writ);

    assert (false);

    LOGERR (klogInt, rc, "KFileRead failed to filter call");

    return rc;
}

#if 0
static
rc_t CC KReencFileWrite (KReencFile *self, uint64_t pos,
                         const void *buffer, size_t bsize,
                         size_t *num_writ)
{
    assert (self);
    assert (buffer);
    assert (bsize);
    assert (num_writ);

    assert (REENCFILE_WRITE_SUPPORTED);

    /* this needs to be finished before we can support Write open */

    return KReencFileWriteUnsupported (self, pos, buffer, bsize, num_writ);
}
#endif


/* ----------------------------------------------------------------------
 * Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KReencFileType (const KReencFile *self)
{
    assert (self != NULL);
    assert (self->encrypted != NULL);

    return KFileType (self->encrypted);
}


/* ----------------------------------------------------------------------
 * KReencFileMake
 *  create a new file object
 */


/* ----------
 * MakeParamValidate
 * common parameter validation for both reencryptors
 */
static
rc_t KReencFileMakeParamValidate (const KFile ** pself, const KFile * encrypted, 
                                  const KKey * deckey, const KKey * enckey)
{
    rc_t rc = 0;

    do
    {
        if (pself == NULL)
        {
            rc = RC (rcFS, rcFile, rcConstructing, rcSelf, rcNull);
            LOGERR (klogErr, rc,
                    "pointer to self NULL when creating "
                    "a re-encryptor");
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

        if ((enckey == NULL) || (deckey == NULL))
        {
            rc = RC (rcFS, rcFile, rcConstructing, rcParam, rcNull);
            LOGERR (klogErr, rc, 
                    "key not supplied when creating a re-encryptor");
            break;
        }

        switch (deckey->type)
        {
        default:
            rc = RC (rcFS, rcFile, rcConstructing, rcParam, rcInvalid);
            PLOGERR (klogErr,
                     (klogErr, rc, "invalid key type '$(T)' should be "
                      "kkeyAES128(1), kkeyAES192(2) or kkeyAES256(3)",
                      "T=%u", deckey->type));
            break;

        case kkeyAES128:
        case kkeyAES192:
        case kkeyAES256:
            break;
        }
        switch (enckey->type)
        {
        default:
            rc = RC (rcFS, rcFile, rcConstructing, rcParam, rcInvalid);
            PLOGERR (klogErr,
                     (klogErr, rc, "invalid key type '$(T)' should be "
                      "kkeyAES128(1), kkeyAES192(2) or kkeyAES256(3)",
                      "T=%u", enckey->type));
            break;

        case kkeyAES128:
        case kkeyAES192:
        case kkeyAES256:
            break;
        }
    } while (0);
    return rc;
}


/* ----------
 * Read mode is fully seekable if the underlying KFile is seekable some
 * integrity checking will not be performed in allowing this seeking.
 */
static const KFile_vt_v1 vtKReencFileRead =
{
    /* version */
    1, 1,

    /* 1.0 */
    KReencFileDestroy,
    KReencFileGetSysFileUnsupported,
    KReencFileRandomAccess,
    KReencFileSize,
    KReencFileSetSizeUnsupported,
    KReencFileRead,
    KReencFileWriteUnsupported,

    /* 1.1 */
    KReencFileType
};


LIB_EXPORT rc_t CC KReencFileMakeRead (const KFile ** pself, 
                                       const KFile * encrypted,
                                       const KKey * deckey,
                                       const KKey * enckey)
{
    KReencFile * self;
    uint64_t size;
    uint64_t block_count;
    rc_t rc;

    rc = KReencFileMakeParamValidate (pself, encrypted, deckey, enckey);
    if (rc)
    {
        LOGERR (klogErr, rc, "error constructing decryptor");
        return rc;
    }

    rc = KFileSize (encrypted, &size);
    if (GetRCState (rc) == rcUnsupported)
    {
        size = 0;
        rc = RC (rcKrypto, rcFile, rcConstructing, rcSize, rcUnsupported);
        LOGERR (klogErr, rc, "Can't re-encrypt files that don't support KFileSize");
        return rc;
    }
    
    if (rc)
    {
        LOGERR (klogErr, rc, "Unable to attempt to size encrypted file for reencryption");
        return rc;
    }

    rc = KFileAddRef (encrypted);
    if (rc)
    {
        LOGERR (klogErr, rc, "Unable to add reference to encrypted file for re-encryptor");
        return rc;
    }

    if (size == 0)
    {
        *pself = encrypted;
        return rc;
    }
    if (size < sizeof (KEncFileHeader) + sizeof (KEncFileFooter))
    {
        rc = RC (rcKrypto, rcFile, rcConstructing, rcSize, rcInvalid);
        LOGERR (klogErr, rc, "encrypted file too short to be valied for re-encryption");
        KFileRelease (encrypted);
        return rc;
    }

    {
        uint64_t temp;

        temp = size - (sizeof (KEncFileHeader) + sizeof (KEncFileFooter));
        block_count = temp / sizeof (KEncFileBlock);
        if ((block_count * sizeof (KEncFileBlock)) != temp)
        {
            rc = RC (rcKrypto, rcFile, rcConstructing, rcSize, rcInvalid);
            LOGERR (klogErr, rc, "encrypted file invalid size for re-encryption");
            KFileRelease (encrypted);
            return rc;
        }
    }

    self = calloc (1,sizeof (*self));

    if (self == NULL)
    {
        rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
        LOGERR (klogSys, rc,
                "out of memory creating encrypter and/or decryptor");
    }
    else
    {
        rc = KFileInit (&self->dad, (const KFile_vt*)&vtKReencFileRead, "KReencFile", "no-name", true, false);
        if (rc)
            LOGERR (klogInt, rc, "failed in initialize reenc base class");
        else
        {
            self->encrypted = encrypted;
            /* dec, enc, ram need to be Make */
            /* num_read, num_write stay 0 */
            self->block_id = NO_CURRENT_BLOCK;
            /* missing needs to be Made */
            /* next_block stays 0 */
            /* seek_block stays 0 - obsolete */
            self->footer_block = EncryptedPos_to_BlockId (size, NULL, NULL);
            self->size = size;
            self->known_size = true; /* obsolete */
            /* plain_text and block stay 0 */
            self->foot.foot.block_count = self->footer_block;

            rc = KEncFileMakeRead (&self->dec, encrypted, deckey);
            if (rc)
                LOGERR (klogErr, rc, "Failed to create re-enc decryptor");

            else
            {
                rc = KRamFileMakeUpdate (&self->ram, self->block.text, 
                                         sizeof self->block.text);
                if (rc)
                    LOGERR (klogErr, rc, 
                            "Failed to create re-enc encryptor");
                else
                {

                    rc = KEncFileMakeWriteBlock (&self->enc, self->ram, enckey);
                    if (rc)
                        LOGERR (klogErr, rc,
                                "Failed to create RAM file for reenc");
                    else
                    {
                        *pself = &self->dad;
                        return 0;
                    }
                    KFileRelease (self->ram);
                }
                KFileRelease (self->dec);
            }
        }
        free (self);
    }
    KFileRelease (encrypted);
    return rc;
}


static const KFile_vt_v1 vtKEncryptFileRead =
{
    /* version */
    1, 1,

    /* 1.0 */
    KReencFileDestroy,
    KReencFileGetSysFileUnsupported,
    KReencFileRandomAccess,
    KEncryptFileSize,
    KReencFileSetSizeUnsupported,
    KReencFileRead,
    KReencFileWriteUnsupported,

    /* 1.1 */
    KReencFileType
};


LIB_EXPORT rc_t CC KEncryptFileMakeRead (const KFile ** pself, 
                                         const KFile * encrypted,
                                         const KKey * enckey)
{
    KReencFile * self;
    uint64_t rawsize;
    uint64_t size;
    rc_t rc;

    rc = KReencFileMakeParamValidate (pself, encrypted, enckey, enckey);
    if (rc)
    {
        LOGERR (klogErr, rc, "error constructing decryptor");
        return rc;
    }

    rc = KFileSize (encrypted, &rawsize);
    if (GetRCState (rc) == rcUnsupported)
    {
        size = 0;
        rc = RC (rcKrypto, rcFile, rcConstructing, rcSize, rcUnsupported);
        LOGERR (klogErr, rc, "Can't encrypt files that don't support KFileSize");
        return rc;
    }
    
    if (rc)
    {
        LOGERR (klogErr, rc, "Unable to attempt to size encrypted file for encryption");
        return rc;
    }

    rc = KFileAddRef (encrypted);
    if (rc)
    {
        LOGERR (klogErr, rc, "Unable to add reference to unencrypted file for encryptor");
        return rc;
    }

    self = calloc (1,sizeof (*self));

    if (self == NULL)
    {
        rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
        LOGERR (klogSys, rc,
                "out of memory creating encrypter and/or decryptor");
    }
    else
    {
        rc = KFileInit (&self->dad, (const KFile_vt*)&vtKEncryptFileRead, "KEncryptFile", "no-path", true, false);
        if (rc)
            LOGERR (klogInt, rc, "failed in initialize reenc base class");
        else
        {
            self->encrypted = encrypted;
            /* dec, enc, ram need to be made below */
            /* num_read, num_write stay 0 */
            self->block_id = NO_CURRENT_BLOCK;
            /* missing needs to be Made */
            /* next_block stays 0 */
            /* seek_block stays 0 - obsolete */

            self->footer_block = PlaintextSize_to_BlockCount ( rawsize, NULL );
            size = BlockId_to_CiphertextOffset ( self -> footer_block ) + sizeof ( KEncFileFooter );
            self->size = size;
            self->known_size = true; /* obsolete */

            /* plain_text and block stay 0 */
            self->foot.foot.block_count = self->footer_block;

            rc = KFileAddRef (self->dec = encrypted);
            if (rc)
                LOGERR (klogErr, rc, "Unable to add reference to unencrypted file for encryptor");

            else
            {
                rc = KRamFileMakeUpdate (&self->ram, self->block.text, 
                                         sizeof self->block.text);
                if (rc)
                    LOGERR (klogErr, rc, 
                            "Failed to create ram file for encryptor");
                else
                {
                    
                    rc = KEncFileMakeWriteBlock (&self->enc, self->ram, enckey);
                    if (rc)
                        LOGERR (klogErr, rc,
                                "Failed to create RAM file for enc");
                    else
                    {
                        *pself = &self->dad;
                        return 0;
                    }
                    KFileRelease (self->ram);
                }
                KFileRelease (self->dec);
            }
        }
        free (self);
    }
    KFileRelease (encrypted);
    return rc;
}


/* ----------
 * Write mode re-encrypted file 
 */
#if 0
static const KFile_vt_v1 vtKReencFileWrite =
{
    /* version */
    1, 1,

    /* 1.0 */
    KReencFileDestroy,
    KReencFileGetSysFileUnsupported,
    KReencFileRandomAccess,
    KReencFileSize,
    KReencFileSetSizeUnsupported,
    KReencFileReadUnsupported,
    KReencFileWrite,

    /* 1.1 */
    KReencFileType
};
#endif

LIB_EXPORT rc_t CC KReencFileMakeWrite (KFile ** pself, 
                                        KFile * encrypted,
                                        const KKey * deckey,
                                        const KKey * enckey)
{
#if REENCFILE_WRITE_SUPPORTED
    KReencFile * self;
    rc_t rc;

    rc = KFileSetSize (encrypted, 0);
#if 0
    if (rc)
        LOGERR (klogWarn, rc, "error truncating output file - "
                "corrupted file might result");
#endif

    rc = KReencFileMakeParamValidate (&self, encrypted, deckey, enckey,
                                      &vtKEncFileWrite, true);
    if (rc)
        LOGERR (klogErr, rc, "error creating encryptor");
    else
        *pself = &self->dad;
    return rc;
#else
    return RC ( rcFS, rcFile, rcCreating, rcFunction, rcUnsupported );
#endif
}


/* end of file reencfile.c */


