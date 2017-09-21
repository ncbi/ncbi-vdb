/*===========================================================================
 *
 *                            Public DOMAIN NOTICE
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
#include <klib/rc.h>
#include <kfs/file.h>
#include <kfs/arc.h>
#include <kfs/toc.h>
#include <kfs/sra.h>
#include <kfs/directory.h>
#include <klib/defs.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <sysalloc.h>

#include "toc-priv.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct KTocFile;
#define KFILE_IMPL struct KTocFile
#include <kfs/impl.h>

#define OPEN_FILE_CACHE_MAX 1


/*--------------------------------------------------------------------------
 * FileCacheNode
 */
typedef struct FileCacheNode FileCacheNode;

struct FileCacheNode
{
    BSTNode n;
    const char *path;
    const KFile *file;
    struct FileCacheNode * newer;
};

static
int64_t CC FileCacheNodeCmp ( const void *item, const BSTNode *n )
{
    const char *a = item;
    const FileCacheNode *b = ( const FileCacheNode* ) n;

    if ( a < b -> path )
        return -1;
    return a > b -> path;
}

static
int64_t CC FileCacheNodeSort ( const BSTNode *item, const BSTNode *n )
{
    const FileCacheNode *a = ( const FileCacheNode* ) item;
    const FileCacheNode *b = ( const FileCacheNode* ) n;

    if ( a -> path < b -> path )
        return -1;
    return a -> path > b -> path;
}

static
void CC FileCacheNodeWhack ( BSTNode *n, void *ignore )
{
    FileCacheNode *self = ( FileCacheNode* ) n;
    KFileRelease ( self -> file );
    free ( self );
}

static
rc_t FileCacheNodeMake ( FileCacheNode **fcnp, const char *path, const KFile *file )
{
    FileCacheNode *fcn = malloc ( sizeof *fcn );
    if ( fcn == NULL )
        return RC ( rcFS, rcFile, rcReading, rcMemory, rcExhausted );

    fcn -> path = path;
    fcn -> file = file;

    * fcnp = fcn;
    return 0;
}

/*--------------------------------------------------------------------------
 * KTocFile
 */

typedef struct KTocFile
{
    KFile dad;
    const KToc * toc;
    const KDirectory * dir;
    uint8_t * header;
/* these three below fields are mutable */
    BSTree open_file_cache;     /* opened files by name */
    FileCacheNode * head;       /* least recently opened */
    FileCacheNode * tail;       /* most recently opened */
/* these three above fields are mutable */
    uint64_t file_size;
    size_t header_size;
/* the below field s mutable */
    uint32_t cache_count;
/* the above field is mutable */
} KTocFile;

/* Destroy
 */
static
rc_t CC KTocFileDestroy (KTocFile *self)
{
    BSTreeWhack ( & self -> open_file_cache, FileCacheNodeWhack, NULL );
    KTocRelease (self->toc);
    KDirectoryRelease (self->dir);
    free (self->header);
    free (self);
    return 0;
}

/* GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 *
 * Since this file does not have a system file underlaying the header
 * nor does it have a single file underlaying the various "subfiles"
 * it must return NULL;
 */
static struct
KSysFile *CC KTocFileGetSysFile ( const KTocFile *self, uint64_t *offset )
{
    * offset = 0;
    return NULL;
}

/* RandomAccess
 *  ALMOST by definition, the file is random access
 *  certain file types ( notably compressors ) will refuse random access
 *
 *  returns 0 if random access, error code otherwise
 */
static
rc_t CC KTocFileRandomAccess ( const KTocFile *self )
{
    assert (self != NULL);
    return 0;
}


/* Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KTocFileType ( const KTocFile *self )
{
    return kfdFile;
}


/* Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t CC KTocFileSize ( const KTocFile *self, uint64_t *size )
{
    assert (self != NULL);
    assert (size != NULL);

    *size = self->file_size;
    return 0;
}

/* SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KTocFileSetSize ( KTocFile *self, uint64_t size )
{
    return RC (rcFS, rcFile, rcUpdating, rcToc, rcUnsupported);
}

/* Read
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
size_t get_filler (const KTocFile *self, size_t z)
{
    size_t align = KTocAlignmentGet((KToc *)self->toc); /* type convert mutable field */
    if (align > 1)
    {
        size_t mask = (size_t)align - 1;    /* get significant bits */
        return (align - (z & mask)) & mask;
    }
    return 0;
}

static
rc_t CC KTocFileRead ( const KTocFile *self, uint64_t _pos,
    void *_buffer, size_t bsize, size_t *num_read )
{
    rc_t rc;
    uint64_t pos;
    uint8_t * buffer;
    size_t filler;

    assert (self != NULL);
    assert (_buffer != NULL);

    TOC_DEBUG (("%s: off %lu siz %zu ------\n", __func__, _pos, bsize));
    TOC_DEBUG (("%s: self->file_size %lu\n", __func__, self->file_size));
    rc = 0;
    pos = _pos;
    buffer = _buffer;

    *num_read = 0;

    /* go ahead and trim from read attempts anything beyond EOF
     * return as EOF if we are asked to read beyond virtual file size
     */
    if (pos > self->file_size)  /* fully past EOF quit now */
    {
        TOC_DEBUG (("KTocFileRead read 0 past EOF"));
	return 0;
    }

    if ((pos + bsize) > self->file_size)        /* partially past EOF quit at EOF */
    {
	bsize = (size_t)( self->file_size - pos );
        TOC_DEBUG (("KTocFileRead trimmed read to %zu\n",bsize));
    }

    filler = get_filler (self, self->header_size);

    /* get portion of read from header */
    if (pos < (uint64_t) (self->header_size + filler))
    {
	size_t to_read = 0;

        TOC_DEBUG (("%s size of header %zu filler after header %zu\n",
                    __func__, self->header_size, filler));

        /* header proper */
        if (pos < self->header_size)
        {
            to_read = self->header_size - (size_t)pos;
            if (to_read > bsize)
                to_read = bsize;
            memmove (buffer, self->header + pos, to_read);

            pos += to_read;
            bsize -= to_read;
        }
        /* filler after header */
        if (pos != self->header_size)    /* didn't reach end of header */
            filler = 0;
        else
        {
            if (filler > bsize)
                filler = bsize;
            if (filler)
            {
                memset (buffer + to_read, 0, filler);
            }
        }

	/* exit on partial read */
	*num_read = to_read + filler;
        TOC_DEBUG (("%s hdr read %zu", __func__, *num_read));
	return 0;
    }
    else
        /* start in on files */
    {
        uint64_t fpos;
        uint64_t fsize;
        const char * path;
        const KDirectory * dir;
        size_t to_read;
        size_t this_read;
        size_t all_read;
        size_t filler;

        *num_read = all_read = to_read = 0;

        dir = KTocGetArchive (self->toc);
        rc = KTocResolvePathFromOffset (self->toc, &path, &fpos, pos);
        if (path != NULL)
        {
            if (rc != 0)
            {
                TOC_DEBUG (("%s can't resolve offset $(O) %R\n", __func__, pos, rc));
                return rc;
            }
            TOC_DEBUG (("%s reading \"%s\" at offset %lu\n", __func__, path, pos));
            rc = KDirectoryFileSize (dir, &fsize, "%s", path);
            if (rc != 0)
            {
                TOC_DEBUG (("%s can't  determine sub file size  %s %R\n", __func__, path, rc));
                return rc;
            }
            TOC_DEBUG (("%s size %lu\n", __func__, fsize));

            if ((fpos + bsize) <= fsize)
            {
                filler = 0;
                to_read = bsize;
            }
            else
            {
                filler = get_filler ( self, (size_t)fsize );
                if ( bsize > ( fsize + filler ) )
                    bsize = (size_t)( fsize + filler );

                if (fpos < fsize)
                {
                    to_read = (size_t)( fsize - fpos );
                    if (bsize <  (to_read + filler))
                        filler = bsize - to_read;
                }
                else
                {
                    to_read = 0;
                    if (filler > bsize)
                        filler = bsize;
                }
            }

            if (to_read)
            {
                KTocFile *mself = (KTocFile *)self;        /* to access mutable fields within self */
                FileCacheNode *n;

                TOC_DEBUG (("%s subfile %s\n", __func__, path));

                /* look in cache */
                n = ( FileCacheNode* ) BSTreeFind ( & mself -> open_file_cache,
                                                    path, FileCacheNodeCmp );
                if ( n != NULL )
                {
                    FileCacheNode * this_one;
                    TOC_DEBUG (("%s cache hit for subfile %s\n", __func__, path));

                    for (this_one = mself->head; this_one != n; this_one = this_one->newer)
                    {
                        if (this_one == NULL)
                            return RC (rcFS, rcFile, rcOpening, rcToc, rcCorrupt);
                    }
                    if (mself->tail != mself->head) /* nothing to do if only one in queue */
                    {
                        if (this_one == mself->head)
                        {
                            mself->head = n->newer;
                            n->newer = NULL;
                            this_one = mself->tail;
                            /* race condition possible: bad if next pair of assigns is not atomic and multi-threading */
                            mself->tail = this_one->newer = n;
                        }
                    }
                }
                else
                {
                    const KFile * file;

                    for (;;)
                    {
                        /* if we don't have too many files open, try to close something in the cache */
                        if (mself->cache_count <= OPEN_FILE_CACHE_MAX)
                        {
                            TOC_DEBUG (("%s open subfile %s\n", __func__, path));
                            /* open file on demand */
                            rc = KDirectoryOpenFileRead (KTocGetArchive(self->toc), &file, "%s", path);
                            if (rc == 0)
                                break;
                        }
                        
                        /* if we've run out of file descriptors try clearing the cache a bit */
                        if ( ( rc == 0 ) || ( ( GetRCObject( rc ) == ( enum RCObject ) rcFileDesc ) &&
                                               ( GetRCState( rc ) == rcExhausted ) ) )
                        {
                            TOC_DEBUG (("%s no more file descriptors\n", __func__));

                            if (mself->cache_count != 0)
                            {
                                FileCacheNode * pn;

                                rc = 0;
                                pn = mself->head;
                                TOC_DEBUG (("%s cache closing %s\n", __func__, pn->path));
                                BSTreeUnlink (&mself->open_file_cache, &pn->n);
                                mself->cache_count--;
                                mself->head = pn->newer;
                                (void)FileCacheNodeWhack (&pn->n, NULL);
                                continue;
                            }
                        }
                        return rc;
                    }

                    /* create a new cache node */
                    rc = FileCacheNodeMake ( & n, path, file );
                    if ( rc != 0 )
                    {
                        TOC_DEBUG (("%s %R  can't cache sub file %s\n", __func__, rc, path));
                        KFileRelease ( file );
                        return rc;
                    }

                    /* insert into cache */
                    BSTreeInsert ( & mself -> open_file_cache, & n -> n, FileCacheNodeSort );
                    mself->cache_count++;
                    if (mself->head == NULL)
                        mself->head = mself->tail = n;
                    else
                    {
                        mself->tail->newer = n;
                        mself->tail = n;
                    }
                }

                rc = KFileRead (n ->file, fpos, buffer, to_read, &this_read);
                if ( rc != 0)
                {
                    TOC_DEBUG (("%s %R error reading sub file %s\n", __func__, rc, path));
                    return rc;
                }
            }
            if (filler)
                memset (buffer+to_read, 0, filler);

            *num_read = to_read + filler;
            TOC_DEBUG (("%s file %s read %zu\n", __func__, path, *num_read));
        }
    }
    return rc;
}

/* Write
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
rc_t CC KTocFileWrite ( KTocFile *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ)
{
    assert ( self != NULL );
    return RC (rcFS, rcFile, rcWriting, rcToc, rcUnsupported);
}


static const KFile_vt_v1 vtKTocFile =
{
    /* version 1.1 */
    1, 1,

    /* start minor version 0 methods */
    KTocFileDestroy,
    KTocFileGetSysFile,
    KTocFileRandomAccess,
    KTocFileSize,
    KTocFileSetSize,
    KTocFileRead,
    KTocFileWrite,
    /* end minor version 0 methods */

    /* start minor version == 1 */
    KTocFileType
    /* end minor version == 1 */
};

LIB_EXPORT rc_t CC KTocFileMake ( const KFile ** pself,
		   const KToc * toc,
		   const KDirectory * dir,
		   void * header,
		   uint64_t file_size,
		   size_t header_size )
{
    KTocFile * self;
    rc_t rc;

    rc = 0;
    self = malloc (sizeof (*self));
    if (self == NULL)
    {
	rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
	LOGERR (klogErr, rc, "Out of memory in KTocFileMake");
    }
    else
    {
	rc = KFileInit (&self->dad, (const KFile_vt*)&vtKTocFile, "KTocFile", "no-name", true, false);
	if (rc != 0)
	{
	    LOGERR (klogErr, rc, "Init failure in KTocFileMake");
	}
	else
	{
	    self->toc = toc;
	    self->dir = dir;
	    self->header = header;
            BSTreeInit ( & self -> open_file_cache );
            self->head = self->tail = NULL;
            self->cache_count = 0;
	    self->file_size = file_size;
	    self->header_size = header_size;
            TOC_DEBUG (("%s: file_size %lu header_size %u\n", __func__, file_size, header_size));
	    KTocAddRef (toc);
	    KDirectoryAddRef (dir);
	    *pself = &self->dad;
	    return 0;
	}
	free (self);
    }
    return rc;
}

LIB_EXPORT rc_t CC KDirectoryOpenTocFileRead ( const KDirectory * self,
				const KFile ** pfile,
                                KSRAFileAlignment align,
				bool ( CC * filter ) ( const KDirectory*, const char*, void* ),
				void * filter_param,
				rc_t ( CC * usort ) ( const KDirectory*, struct Vector* ) )
{
    rc_t rc;

    TOC_DEBUG (("%s: enter\n",__func__));

    rc = 0;
    if (self == NULL)
    {
	rc = RC (rcFS, rcDirectory, rcConstructing, rcSelf, rcNull);
	LOGERR (klogErr, rc, "Directory NULL in constructing a TocFile");
	return rc;
    }
    if (pfile == NULL)
    {
	rc = RC (rcFS, rcDirectory, rcConstructing, rcParam, rcNull);
	LOGERR (klogErr, rc, "file pointer parameter NULL in constructing a TocFile");
	return rc;
    }
    if (rc == 0)
    {
	const KDirectory * dir;

	rc = KDirectoryOpenArcDirRead (self, &dir, true, ".", tocKDirectory,
				       KArcParseKDir, filter, filter_param);
	if (rc != 0)
	{
	    LOGERR (klogErr, rc, "Failure to parse directory to TOC");
	}
	else
	{
	    void * header;
	    uint64_t file_size;
	    size_t header_size;

            TOC_DEBUG (("%s: Persisting header\n", __func__));
	    rc = KArcDirPersistHeader ((KArcDir*)dir, &header, &header_size, &file_size, align, usort);
	    if (rc == 0)
	    {
		const KToc * toc;

                TOC_DEBUG (("%s: Get TOC\n", __func__));
		rc = KArcDirGetTOC ((const KArcDir*)dir, &toc); /* does not addref() */
		if (rc != 0)
		{
		    LOGERR (klogErr, rc, "Failure to parse directory to TOC");
		}
		else
		{
		    const KFile * file;

                    TOC_DEBUG (("%s: call KTocFileMake file_size %lu \n", __func__, file_size));

		    rc = KTocFileMake (&file, toc, self, header, file_size, header_size);
		    if (rc != 0)
		    {
			LOGERR (klogErr, rc, "Failure to make KTocFile");
		    }
		    else
		    {
			*pfile = file;
		    }
		}
	    }
            KDirectoryRelease (dir);
	}
    }
    return rc;
}
						

/* end of file dirfile.c */
