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

#include <kfs/extern.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <kfs/file.h>
#include <sysalloc.h>
#include <kfs/ramfile.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* ======================================================================
 * KRamFile
 */

/* -----
 * define the specific types to be used in the templatish/inheritancish
 * definition of vtables and their elements
 */
#define KFILE_IMPL struct KRamFile
#include <kfs/impl.h>

/*-----------------------------------------------------------------------
 * KRamFile
 *  A KFile that puts reads and writes into a buffer. The buffer is passed
 *  in so the creator will know where the buffer is.
 */
struct KRamFile
{
    KFile	dad;

    uint64_t pos;
    uint64_t max_pos;
    size_t   bsize;
    size_t   max;
    char *   buffer;
};

/* ----------------------------------------------------------------------
 * Destroy
 *
 */
static
rc_t CC KRamFileDestroy (KRamFile *self)
{
    free (self);
    return 0;
}

/* ----------------------------------------------------------------------
 * GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or RAM if
 *  no such file is available.
 *
 * We cant allow memory mapping a tee file as the read?writes ar needed
 * to trigger the writes to the copy KFile
 */

static
struct KSysFile *CC KRamFileGetSysFile (const KRamFile *self, uint64_t *offset)
{
    /* parameters must be non-RAM */
    assert (self != NULL);
    assert (offset != NULL);

    return NULL;
}

/* ----------------------------------------------------------------------
 * RandomAccess
 *
 *  returns 0 if random access, error code otherwise
 *
 * Update needs to be able to seek both original and copy while read
 * only needs to be able to seek the original.
 */
static
rc_t CC KRamFileRandomAccess (const KRamFile *self)
{
    assert (self != NULL);
    return 0;
}

/* ----------------------------------------------------------------------
 * Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KRamFileType (const KRamFile *self)
{
    return kfdNull;
}


/* ----------------------------------------------------------------------
 * Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t CC KRamFileSize (const KRamFile *self, uint64_t *size)
{
    assert (self != NULL);
    assert (size != NULL);

/*     return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported ); */
    *size = self->max_pos;
    return 0;
}

/* ----------------------------------------------------------------------
 * SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KRamFileSetSize (KRamFile *self, uint64_t size)
{
/*     return RC (rcFS, rcFile, rcUpdating, rcSelf, rcUnsupported); */
    self->max_pos = size;
    return 0;
}

/* ----------------------------------------------------------------------
 * Read
 *  read file from known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT, RAM OKAY ] - optional return parameter
 *  giving number of bytes actually read
 *
 * NOTE: Can not seek;
 *
 */
static
rc_t CC KRamFileRead	(const KRamFile *self,
			 uint64_t pos,
			 void *buffer,
			 size_t bsize,
			 size_t *num_read)
{
    assert (self);
    assert (buffer);
    assert (num_read);
    /* no restrictions on pos or bsize */

    if (pos >= self->max_pos)
    {
        *num_read = 0;
        return 0;
    }

    if (pos + bsize > self->max_pos)
        bsize = self->max_pos - pos;

    /* if we are trying to read outside the current window */
    if (self->pos > pos)
    {
        /* get some zeros */
        size_t left_zeroes;

        left_zeroes = self->pos - pos;

        if (left_zeroes >= bsize)
            left_zeroes = bsize;

        memset (buffer, 0, bsize);
    }
    else if (self->pos + self->bsize < pos)
        memset (buffer, 0, bsize);

    /* so we are inside the current window */
    else
    {
        /* are we trying to go past the window? */
        if (pos + bsize > self->pos + self->bsize)
            bsize = (self->pos + self->bsize) - pos;

        memmove (buffer, self->buffer + (pos - self->pos), bsize);
    }
    *num_read = bsize;
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
 *  "num_writ" [ OUT, RAM OKAY ] - optional return parameter
 *  giving number of bytes actually written
 *
 * Always accepts writes and can seek.
 */
static
rc_t CC KRamFileWrite (KRamFile *self, uint64_t pos,
		       const void *buffer, size_t bsize,
		       size_t *num_writ)
{
    char * to;

    /* can't really fail */

    assert (self);
    assert (buffer);
    assert (num_writ);

    /* does the write fill the buffer? */
    if (bsize >= self->max)
    {
        /* we can never allow a write larger than the internal buffer */
        self->pos = pos;
        self->bsize = bsize = self->max;
        to = self->buffer;
        if (self->max_pos < pos + bsize)
            self->max_pos = pos + bsize;
    }

    /* else are we seeking forward or backward to completely new or completely
     * over writing what is there */
    else if ((self->bsize == 0) ||                     /* currently no buffer */
             (pos >= (self->pos + self->max)) || /* past current */
             (pos + self->max <= self->pos) ||   /* behind current */
             ((pos <= self->pos) && (pos + bsize >= self->pos + self->bsize)))
    {
        /* reset the buffer parameters */
        self->pos = pos;
        self->bsize = bsize;
        if (self->max_pos < pos + bsize)
            self->max_pos = pos + bsize;

        to = self->buffer;
        /* clear any unused portion of the buffer */
        if (self->max > bsize)
            memset (self->buffer + bsize, 0, self->max - bsize);
    }
    else if (pos >= self->pos) 
    {
        /* if we are reading something at or after what we have now
         * where the right edge was doesn't matter since we zero fill
         * in the previous condition and at construction */
        /* how far into the buffer will we start */
        size_t poff = pos - self->pos;

        /* do we have to slide the window? This is not optimized */
        if ((poff != 0) && (poff + bsize > self->max))
        {
            size_t m = poff + bsize - self->max;
            if (self->bsize < self->max)
                memset (self->buffer + bsize, 0, self->max = self->bsize);
            /* must use memmove not memmove */
            memmove (self->buffer, self->buffer + poff, self->max - pos);

            poff -= m;
            self->pos -= m;
        }

        /* check for a new self->bsize */
        if (poff + bsize > self->bsize)
            self->bsize = poff + bsize;

        /* check for a new max position */
        if (self->max_pos < self->pos + self->bsize)
            self->max_pos = self->pos + self->bsize;

        to = self->buffer + poff;
    }
    else
    {
        size_t poff = self->pos - pos;

        /* non optimized slide */
        memmove (self->buffer + poff, self->buffer, self->max - poff);
        self->pos = pos;
        self->bsize += poff;
        if (self->bsize > self->max)
            self->bsize = self->max;
        to = self->buffer;
    }
    memmove (to, buffer, bsize);
    *num_writ = bsize;

    return 0;
}


static const KFile_vt_v1 vtKRamFile =
{
    /* version */
    1, 1,

    /* 1.0 */
    KRamFileDestroy,
    KRamFileGetSysFile,
    KRamFileRandomAccess,
    KRamFileSize,
    KRamFileSetSize,
    KRamFileRead,
    KRamFileWrite,

    /* 1.1 */
    KRamFileType
};

/* ----------------------------------------------------------------------
 * KRamFileMake
 *  create a new file object
 */
static
rc_t KRamFileMake (KRamFile ** pself, char * buffer, size_t buffer_size, bool r, bool w)
{
    rc_t	rc;
    KRamFile *	self;

    /* -----
     */
    if (pself == NULL)
        return RC (rcFS, rcFile, rcConstructing, rcSelf, rcNull);

    if (buffer == NULL)
        return RC (rcFS, rcFile, rcConstructing, rcParam, rcNull);

    /* we only deny size 0 buffer just to avoid the too weird corner cases */
    if (buffer_size == 0)
        return RC (rcFS, rcFile, rcConstructing, rcParam, rcTooShort);

    if (((r != true) && (r != false)) || ((w != true) && (w != false)))
        return RC (rcFS, rcFile, rcConstructing, rcParam, rcInvalid);

    /* -----
     * get space for the object
     */
    self = malloc (sizeof (*self));
    if (self == NULL)	/* allocation failed */
    {
	/* fail */
	rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
    }
    else
    {
	rc = KFileInit (&self->dad,			/* initialize base class */
			(const KFile_vt*)&vtKRamFile, 	/* VTable for KRamFile */
            "KRamFile", "ram",
                        r,w);
	if (rc == 0)
	{
            self->pos = self->max_pos = 0;
            self->max = buffer_size;
            self->bsize = 0;
            self->buffer = buffer;
	    *pself = self;
	    return 0;
	}
	/* fail */
	free (self);
    }
    return rc;
}


LIB_EXPORT rc_t CC KRamFileMakeRead (const KFile ** selfp,char * buffer, size_t buffer_size)
{
    KRamFile *self;
    rc_t rc = KRamFileMake ( & self, buffer, buffer_size, true, false);
    if ( rc == 0 )
    {
        self -> max_pos = buffer_size;
        self -> bsize = buffer_size;
        * selfp = & self -> dad;
    }
    return rc;
}

LIB_EXPORT rc_t CC KRamFileMakeWrite (KFile ** self, char * buffer, size_t buffer_size)
{
    return KRamFileMake ((KRamFile **)self, buffer, buffer_size, false, true);
}

LIB_EXPORT rc_t CC KRamFileMakeUpdate (KFile ** self, char * buffer, size_t buffer_size)
{
    return KRamFileMake ((KRamFile **)self, buffer, buffer_size, true, true);
}


/* end of file ramfile.c */

