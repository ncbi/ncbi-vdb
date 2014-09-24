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
#include <kfs/nullfile.h>

#include <assert.h>
#include <stdlib.h>

/* ======================================================================
 * KNullFile
 */

/* -----
 * define the specific types to be used in the templatish/inheritancish
 * definition of vtables and their elements
 */
#define KFILE_IMPL struct KNullFile
#include <kfs/impl.h>

static rc_t CC KNullFileDestroy (KNullFile *self);
static struct KSysFile *CC KNullFileGetSysFile (const KNullFile *self,
					     uint64_t *offset);
static rc_t CC KNullFileRandomAccess (const KNullFile *self);
static uint32_t CC KNullFileType (const KNullFile *self);
static rc_t CC KNullFileSize (const KNullFile *self, uint64_t *size);
static rc_t CC KNullFileSetSize (KNullFile *self, uint64_t size);
static rc_t CC KNullFileRead (const KNullFile *self, uint64_t pos,
			   void *buffer, size_t bsize, size_t *num_read);
static rc_t CC KNullFileWrite (KNullFile *self, uint64_t pos, const void *buffer,
			    size_t size, size_t *num_writ);


static const KFile_vt_v1 vtKNullFile =
{
    /* version */
    1, 1,

    /* 1.0 */
    KNullFileDestroy,
    KNullFileGetSysFile,
    KNullFileRandomAccess,
    KNullFileSize,
    KNullFileSetSize,
    KNullFileRead,
    KNullFileWrite,

    /* 1.1 */
    KNullFileType
};

/*-----------------------------------------------------------------------
 * KNullFile
 *  an archive file including tar and sra
 */
struct KNullFile
{
    KFile	dad;
};

/* ----------------------------------------------------------------------
 * KNullFileMake
 *  create a new file object
 */
static
rc_t KNullFileMake (KNullFile ** self, bool write_allowed)
{
    rc_t	rc;
    KNullFile *	pF;

    /* -----
     */
    assert (self != NULL);

    /* -----
     * get space for the object
     */
    pF = malloc (sizeof (KNullFile));
    if (pF == NULL)	/* allocation failed */
    {
	/* fail */
	rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
    }
    else
    {
	rc = KFileInit (&pF->dad,			/* initialize base class */
			(const KFile_vt*)&vtKNullFile, 	/* VTable for KNullFile */
            "KNullFile", "no-name",
			true,				/* read allowed */
			write_allowed);
	if (rc == 0)
	{
	    *self = pF;
	    return 0;
	}
	/* fail */
	free (pF);
    }
    return rc;
}


LIB_EXPORT rc_t CC KFileMakeNullRead ( const KFile ** self )
{
    return KNullFileMake ((KNullFile **)self, false);
}

LIB_EXPORT rc_t CC KFileMakeNullUpdate ( KFile ** self )
{
    return KNullFileMake ((KNullFile **)self, true);
}

/* ----------------------------------------------------------------------
 * Destroy
 *
 */
static
rc_t CC KNullFileDestroy (KNullFile *self)
{
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
struct KSysFile *CC KNullFileGetSysFile (const KNullFile *self, uint64_t *offset)
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
rc_t CC KNullFileRandomAccess (const KNullFile *self)
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
uint32_t CC KNullFileType (const KNullFile *self)
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
rc_t CC KNullFileSize (const KNullFile *self, uint64_t *size)
{
    assert (self != NULL);
    assert (size != NULL);

    *size = 0;
    return 0;;
}

/* ----------------------------------------------------------------------
 * SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KNullFileSetSize (KNullFile *self, uint64_t size)
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
rc_t CC KNullFileRead	(const KNullFile *self,
			 uint64_t pos,
			 void *buffer,
			 size_t bsize,
			 size_t *num_read)
{
    *num_read = 0;
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
rc_t CC KNullFileWrite (KNullFile *self, uint64_t pos,
		       const void *buffer, size_t bsize,
		       size_t *num_writ)
{
    *num_writ = bsize;
    return 0;
}

/* end of file nullfile.c */

