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
#include <klib/debug.h>
#include <klib/rc.h>
#include <kfs/file.h>
#include <kfs/readheadfile.h>
#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
/* ======================================================================
 * KReadHeadFile
 */

/* -----
 * define the specific types to be used in the templatish/inheritancish
 * definition of vtables and their elements
 */
struct KReadHeadFile;
#define KFILE_IMPL struct KReadHeadFile
#include <kfs/impl.h>


/*-----------------------------------------------------------------------
 * KReadHeadFile
 */
typedef struct KReadHeadFile
{
    KFile	  dad;
    uint64_t	  max_position;
    const KFile * original;
    size_t        buffer_size;
    uint8_t       buffer [1];
} KReadHeadFile;

/* ----------------------------------------------------------------------
 * Destroy
 *
 */
static
rc_t CC KReadHeadFileDestroy (KReadHeadFile *self)
{
    rc_t rc;

    rc = KFileRelease (self->original);

    free (self);
    return rc;
}

/* ----------------------------------------------------------------------
 * GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 *
 * bytes could not be counted if memory mapped so this is disallowed
 */

static
struct KSysFile *CC KReadHeadFileGetSysFile (const KReadHeadFile *self, uint64_t *offset)
{
    /* parameters must be non-NULL */
    assert (self != NULL);
    assert (offset != NULL);

    return KFileGetSysFile (self->original, offset);
}

/* ----------------------------------------------------------------------
 * RandomAccess
 *
 *  returns 0 if random access, error code otherwise
 */
static
rc_t CC KReadHeadFileRandomAccess (const KReadHeadFile *self)
{
    return KFileRandomAccess (self->original);
}

/* ----------------------------------------------------------------------
 * Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KReadHeadFileType (const KReadHeadFile *self)
{
    return KFileType (self->original);
}

/* ----------------------------------------------------------------------
 * Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t CC KReadHeadFileSize (const KReadHeadFile *self, uint64_t *size)
{
    return KFileSize (self->original, size);
}

/* ----------------------------------------------------------------------
 * SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KReadHeadFileSetSize (KReadHeadFile *self, uint64_t size)
{
    return RC (rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported);
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
rc_t CC KReadHeadFileRead	(const KReadHeadFile *cself,
				 uint64_t pos,
				 void *buffer,
				 size_t bsize,
				 size_t *num_read)
{
    rc_t rc;
    size_t valid = 0;
    size_t this_read;
    KReadHeadFile *self = (KReadHeadFile *)cself;
    *num_read = 0;

    /* are we re-reading? */
    if (pos < self->max_position)
    {
        size_t valid = self->max_position - pos;

        if (bsize <= valid)
        {
            memmove (buffer, self->buffer + pos, bsize);
            return 0;
        }

        memmove (buffer, self->buffer + pos, valid);
    }
    rc = KFileRead (self->original, pos + valid, (char *)buffer + valid,
                    bsize - valid, &this_read);

    *num_read = (rc) ? 0 : valid + this_read;
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
 *
 * Unsupported as we now treat archives as READ ONLY
 */
static
rc_t CC KReadHeadFileWrite (KReadHeadFile *self, uint64_t pos,
			   const void *buffer, size_t bsize,
			   size_t *num_writ)
{
    assert ( self != NULL );
    return RC (rcFS, rcFile, rcWriting, rcFunction, rcUnsupported);
}

static const KFile_vt_v1 vtKReadHeadFile =
{
    /* version */
    1, 1,

    /* 1.0 */
    KReadHeadFileDestroy,
    KReadHeadFileGetSysFile,
    KReadHeadFileRandomAccess,
    KReadHeadFileSize,
    KReadHeadFileSetSize,
    KReadHeadFileRead,
    KReadHeadFileWrite,

    /* 1.1 */
    KReadHeadFileType
};



/* ----------------------------------------------------------------------
 * KReadHeadFileMake
 *  create a new file object
 */

LIB_EXPORT rc_t CC KFileMakeReadHead (const KFile ** pself,
                                      const KFile * original,
                                      size_t buffer_size)
{
    rc_t rc;

    if (pself == NULL)
        return RC (rcFS, rcFile, rcConstructing, rcSelf, rcNull);

    *pself = NULL;

    if (original == NULL)
        return RC (rcFS, rcFile, rcConstructing, rcParam, rcNull);

    rc = KFileAddRef (original);
    if (rc)
        return rc;

    rc = KFileRandomAccess (original);

    /* buffering not needed as existing file is random access */
    if (rc == 0)
    {
        *pself = original;
        return 0;
    }

    /* some other failure we'll have to bail on */
    else if (GetRCState(rc) != rcUnsupported)
        return rc;

    else
    {
        KReadHeadFile * self;

        self = malloc (sizeof (*self) + buffer_size); /* really too much but too hard to tune down accurately */
        if (self == NULL)
            return RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);

	rc = KFileInit (&self->dad,			/* initialize base class */
			(const KFile_vt*)&vtKReadHeadFile,/* VTable for KReadHeadFile */
            "KReadHeadFile", "no-name",
			original->read_enabled,
			false);
	if (rc == 0)
	{
            self->max_position = 0;
            self->original = original;
            self->buffer_size = buffer_size;
            *pself = &self->dad;
            return 0;
	}
	/* fail */
	free (self);
    }
    return rc;
}

/* end of file countfile.c */

