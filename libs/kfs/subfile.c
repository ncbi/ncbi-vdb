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
#include <kfs/subfile.h>
#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h>

/* ======================================================================
 * KSubFile
 *  a file inside an archive
 */

/* -----
 * define the specific types to be used in the templatish/inheritancish
 * definition of vtables and their elements
 */
#define KFILE_IMPL struct KSubFile
#include <kfs/impl.h>

static rc_t CC KSubFileDestroy (KSubFile *self);
static struct KSysFile *CC KSubFileGetSysFile (const KSubFile *self,
					    uint64_t *offset);
static rc_t CC KSubFileRandomAccess (const KSubFile *self);
static uint32_t CC KSubFileType (const KSubFile *self);
static rc_t CC KSubFileSize (const KSubFile *self, uint64_t *size);
static rc_t CC KSubFileSetSize (KSubFile *self, uint64_t size);
static rc_t CC KSubFileRead (const KSubFile *self, uint64_t pos,
			  void *buffer, size_t bsize, size_t *num_read);
static rc_t CC KSubFileWrite (KSubFile *self, uint64_t pos, const void *buffer,
			   size_t size, size_t *num_writ);


static const KFile_vt_v1 vtKSubFile =
{
    /* version */
    1, 1,

    /* 1.0 */
    KSubFileDestroy,
    KSubFileGetSysFile,
    KSubFileRandomAccess,
    KSubFileSize,
    KSubFileSetSize,
    KSubFileRead,
    KSubFileWrite,

    /* 1.1 */
    KSubFileType
};

/*-----------------------------------------------------------------------
 * KSubFile
 *  an archive file including tar and sra
 */
struct KSubFile
{
    KFile	dad;
    uint64_t	start;
    uint64_t	size;
    KFile *	original;
};

/* ----------------------------------------------------------------------
 * KSubFileMake
 *  create a new file object
 */

static
rc_t KSubFileMake (KSubFile ** self,
		   KFile * original,
		   uint64_t start,
		   uint64_t size,
		   bool read_enabled,
		   bool write_enabled)
{
    rc_t	rc;
    KSubFile *	pF;

    /* -----
     */
    assert (self != NULL);
    assert (original != NULL);
/*     assert (start >= size); */

    /* -----
     * the enables should be true or false
     */
    assert ((read_enabled == true)||(read_enabled == false));
    assert ((write_enabled == true)||(write_enabled == false));

    /* -----
     * get space for the object
     */
    pF = malloc (sizeof (KSubFile));
    if (pF == NULL)	/* allocation failed */
    {
        /* fail */
        rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
    }
    else
    {
        rc = KFileInit (&pF->dad,			/* initialize base class */
            (const KFile_vt*)&vtKSubFile, 	/* VTable for KSubFile */
            "KSubFile", "no-name",
                        read_enabled,		/* read allowed */
                        write_enabled);		/* write disallowed */
	if (rc == 0)
	{
	    KFileAddRef (original);
	    /* succeed */
	    pF->original = original;
	    pF->start = start;
	    pF->size = size;
	    *self = pF;
	    return 0;
	}
	/* fail */
	free (pF);
    }
    return rc;
}

LIB_EXPORT rc_t CC KFileMakeSubRead (const KFile ** self, const KFile * original, uint64_t start, uint64_t size)
{
    return KSubFileMake ((KSubFile **)self, (KFile*)original, start, size,
			 true, false);
}

LIB_EXPORT rc_t CC KFileMakeSubUpdate (KFile ** self, KFile * original, uint64_t start, uint64_t size)
{
    return KSubFileMake ((KSubFile **)self, original, start, size,
			 true, true);
}

/* ----------------------------------------------------------------------
 * Destroy
 *
 */
static
rc_t CC KSubFileDestroy (KSubFile *self)
{
    assert (self != NULL);
    KFileRelease (self->original);
    free (self);
    return 0;
}

/* ----------------------------------------------------------------------
 * GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 *
 * We cant allow memory mapping a tee file as the read?writes ar needed
 * to trigger the writes to the copy KFile
 */

static
struct KSysFile *CC KSubFileGetSysFile (const KSubFile *self, uint64_t *offset)
{
    /* parameters must be non-NULL */
    assert (self != NULL);
    assert (offset != NULL);

    /* not implmenting at this time */
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
rc_t CC KSubFileRandomAccess (const KSubFile *self)
{
    assert (self != NULL);
    return KFileRandomAccess (self->original);
}

/* ----------------------------------------------------------------------
 * Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KSubFileType (const KSubFile *self)
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
rc_t CC KSubFileSize (const KSubFile *self, uint64_t *size)
{
    assert (self != NULL);
    assert (size != NULL);

    *size = self->size;

    return 0;;
}

/* ----------------------------------------------------------------------
 * SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KSubFileSetSize (KSubFile *self, uint64_t size)
{
    return RC (rcFS, rcFile, rcUpdating, rcSelf, rcUnsupported);
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
rc_t CC KSubFileRead	(const KSubFile *self,
			 uint64_t pos,
			 void *buffer,
			 size_t bsize,
			 size_t *num_read)
{
    size_t	request;
    rc_t	rc;


    /* -----
     * self and buffer were validated as not NULL before calling here
     *
     * So get the KTTOCNode type: chunked files and contiguous files 
     * are read differently.
     */
    assert (self != NULL);
    assert (buffer != NULL);
    assert (num_read != NULL);

    *num_read = 0;
    if (pos >= self->size)
    {
	return 0;
    }
    if ((pos + bsize) <= self->size)
	request = bsize;
    else
	request = (size_t)(self->size - pos);

    rc = KFileRead (self->original, self->start + pos, buffer, request, num_read);
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
rc_t CC KSubFileWrite (KSubFile *self, uint64_t pos,
		       const void *buffer, size_t bsize,
		       size_t *num_writ)
{
    size_t to_write;

    assert (self != NULL);
    assert (buffer != NULL);
    assert (num_writ != NULL);

    *num_writ = 0;
    if (pos >= ( uint64_t ) self->size)
        return 0;

    to_write = bsize;
    if ((pos + bsize) > self->size)
        to_write = ( uint64_t ) self -> size - pos;

    return KFileWrite (self->original, self->start + pos, buffer, to_write, num_writ);
}

/* end of file subfile.c */

