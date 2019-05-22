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
#include <kfs/appendfile.h>

#include <assert.h>
#include <stdlib.h>

/* ======================================================================
 * KAppendFile
 *
 * Not really append file, but some kind of
 */

#define KFILE_IMPL struct KAppendFile
#include <kfs/impl.h>

    /* start minor version == 0 */
static rc_t CC KAppendFileDestroy (
                                KAppendFile * self
                                );
static struct KSysFile *CC KAppendFileGetSysFile (
                                const KAppendFile * self,
                                uint64_t * offset
                                );
static rc_t CC KAppendFileRandomAccess (
                                const KAppendFile * self
                                );
static rc_t CC KAppendFileSize (
                                const KAppendFile * self,
                                uint64_t * size
                                );
static rc_t CC KAppendFileSetSize (
                                KAppendFile * self,
                                uint64_t size
                                );
static rc_t CC KAppendFileRead (
                                const KAppendFile * self,
                                uint64_t pos,
			                    void * buffer,
			                    size_t bsize,
			                    size_t * num_read
			                    );
static rc_t CC KAppendFileWrite (
                                KAppendFile *self,
                                uint64_t pos,
                                const void *buffer,
			                    size_t size,
			                    size_t *num_writ
			                    );

    /* start minor version == 1 */
static uint32_t CC KAppendFileType (
                                const KAppendFile * self
                                );
    /* end minor version == 1 */

/* start minor version == 2 */
static rc_t CC KAppendFileTimedRead (
                                const KAppendFile * self,
                                uint64_t pos,
                                void * buffer,
                                size_t bsize,
                                size_t * num_read,
                                struct timeout_t * tm
                                );
static rc_t CC KAppendFileTimedWrite (
                                KAppendFile * self,
                                uint64_t pos,
                                const void * buffer,
                                size_t size,
                                size_t * num_writ,
                                struct timeout_t * tm
                                );
    /* end minor version == 2 */

static const KFile_vt_v1 vtKAppendFile =
{
    /* version */
    1, 1,

    /* 1.0 */
    KAppendFileDestroy,
    KAppendFileGetSysFile,
    KAppendFileRandomAccess,
    KAppendFileSize,
    KAppendFileSetSize,
    KAppendFileRead,
    KAppendFileWrite,

    /* 1.1 */
    KAppendFileType,

    /* 1.2 */
    KAppendFileTimedRead,
    KAppendFileTimedWrite
};

/*-----------------------------------------------------------------------
 * KAppendFile
 *  an archive file including tar and sra
 */
struct KAppendFile
{
    KFile	dad;

    KFile  * original;
    uint64_t original_size;
};

/* ----------------------------------------------------------------------
 * KAppendFileMake
 *  create a new file object
 */
LIB_EXPORT rc_t CC KFileMakeAppend (
                                        KFile ** self,
                                        struct KFile * original
)
{
    rc_t	rc;
    KAppendFile * pF;
    uint64_t original_size;

    /* -----
     */
    assert (self != NULL);

    /* -----
     */
    * self = NULL;

    /* -----
     */
    if ( original == NULL )
    {
	/* fail */
	return RC (rcFS, rcFile, rcConstructing, rcParam, rcNull);
    }

	if ( ! original -> write_enabled )
	{
	/* fail */
	return RC (rcFS, rcFile, rcConstructing, rcParam, rcInvalid);
	}

	rc = KFileSize ( original, & original_size );
	if ( rc != 0 )
	{
	/* fail */
	return RC (rcFS, rcFile, rcConstructing, rcParam, rcInvalid);
	}

	rc = KFileAddRef ( original );
	if ( rc != 0 )
	{
	/* fail */
	return RC (rcFS, rcFile, rcConstructing, rcParam, rcInvalid);
	}

    /* -----
     * get space for the object
     */
    pF = malloc (sizeof (KAppendFile));
    if (pF == NULL)	/* allocation failed */
    {
	/* fail */
	rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
    }
    else
    {
	rc = KFileInit (
	            &pF->dad,			/* initialize base class */
			    (const KFile_vt*)&vtKAppendFile, 	/* VTable for KAppendFile */
                "KAppendFile",
                "no-name",
			    true,				/* read allowed */
			    original -> write_enabled
			    );
	if (rc == 0)
	{
	    pF -> original = original;
	    pF -> original_size = original_size;

	    *self = ( KFile * ) pF;
	    return 0;
	}
	/* fail */
	free (pF);
    }
    return rc;
}

/* ----------------------------------------------------------------------
 * Destroy
 *
 */
static
rc_t CC KAppendFileDestroy (KAppendFile *self)
{
    if ( self != NULL ) {
        if ( self -> original != NULL ) {
            KFileRelease ( self -> original );
            self -> original = NULL;
        }

        free (self);
    }
    return 0;
}

/* ----------------------------------------------------------------------
 * GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 */

static
struct KSysFile *CC KAppendFileGetSysFile (const KAppendFile *self, uint64_t *offset)
{
    struct KSysFile * sys_file;
    uint64_t sys_offset;

    /* parameters must be non-NULL */
    assert (self != NULL);
    assert (offset != NULL);
    assert ( self -> original != NULL );

    /* we should transform something */
    sys_file = KFileGetSysFile ( self -> original, & sys_offset );

    if ( sys_file != NULL ) {
        * offset =
                self -> original_size < sys_offset
                        ? ( sys_offset - self -> original_size )
                        : 0
                        ;
    }

    return sys_file;
}

/* ----------------------------------------------------------------------
 * RandomAccess
 *
 *  returns 0 if random access, error code otherwise
 */
static
rc_t CC KAppendFileRandomAccess (const KAppendFile *self)
{
    assert (self != NULL);
    assert ( self -> original != NULL );

    return KFileRandomAccess ( self -> original );
}

/* ----------------------------------------------------------------------
 * Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KAppendFileType (const KAppendFile *self)
{
    assert (self != NULL);
    assert ( self -> original != NULL );

    return KFileType ( self -> original );
}


/* ----------------------------------------------------------------------
 * Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t CC KAppendFileSize (const KAppendFile *self, uint64_t *size)
{
    rc_t rc;
    uint64_t original_size;

    assert (self != NULL);
    assert (size != NULL);
    assert ( self -> original != NULL );

    rc = KFileSize ( self -> original, & original_size );

    *size = original_size - self -> original_size;

    return rc;
}

/* ----------------------------------------------------------------------
 * SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KAppendFileSetSize (KAppendFile *self, uint64_t size)
{
    assert (self != NULL);
    assert ( self -> original != NULL );

    return KFileSetSize (
                        self -> original,
                        size + self -> original_size
                        );
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
rc_t CC KAppendFileRead	(const KAppendFile *self,
			 uint64_t pos,
			 void *buffer,
			 size_t bsize,
			 size_t *num_read)
{
    assert (self != NULL);
    assert ( self -> original != NULL );

    return KFileRead (
                    self -> original,
                    pos + self -> original_size,
                    buffer,
                    bsize,
                    num_read
                    );
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
rc_t CC KAppendFileWrite (KAppendFile *self, uint64_t pos,
		       const void *buffer, size_t bsize,
		       size_t *num_writ)
{
    assert (self != NULL);
    assert ( self -> original != NULL );

    return KFileWrite (
                    self -> original,
                    pos + self -> original_size,
                    buffer,
                    bsize,
                    num_writ
                    );
}

/*
 * TimedRead
 *  read from file until "bytes" from known position
 *  or return incomplete transfer error
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - number of bytes actually read
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
static
rc_t CC  KAppendFileTimedRead (
                                const KAppendFile * self,
                                uint64_t pos,
                                void * buffer,
                                size_t bsize,
                                size_t * num_read,
                                struct timeout_t * tm
)
{
    assert (self != NULL);
    assert ( self -> original != NULL );

    return KFileTimedRead (
                            self -> original,
                            pos + self -> original_size,
                            buffer,
                            bsize,
                            num_read,
                            tm
                            );
}

/*
 * TimedWrite
 *  write to from file from known position
 *  or return incomplete transfer error
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_writ" [ OUT ] - number of bytes actually read
 *
 *  "tm" [ IN/OUT, NULL OKAY ] - an optional indicator of
 *  blocking behavior. not all implementations will support
 *  timed reads. a NULL timeout will block indefinitely,
 *  a value of "tm->mS == 0" will have non-blocking behavior
 *  if supported by implementation, and "tm->mS > 0" will indicate
 *  a maximum wait timeout.
 */
static
rc_t CC KAppendFileTimedWrite (
                                KAppendFile * self,
                                uint64_t pos,
                                const void * buffer,
                                size_t bsize,
                                size_t * num_writ,
                                struct timeout_t * tm
)
{
    assert (self != NULL);
    assert ( self -> original != NULL );

    return KFileTimedWrite (
                            self -> original,
                            pos + self -> original_size,
                            buffer,
                            bsize,
                            num_writ,
                            tm
                            );
}

/* end of file appendfile.c */

