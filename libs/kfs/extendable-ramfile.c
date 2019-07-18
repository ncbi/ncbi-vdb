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

#include <kfs/extendable-ramfile.h>
#include <kfs/file.h>

#include <klib/rc.h>

#include <sysalloc.h>

#include <string.h>

/* ======================================================================
 * KExtendableRamFile
 */

/* -----
 * define the specific types to be used in the templatish/inheritancish
 * definition of vtables and their elements
 */
typedef struct KExtendableRamFile KExtendableRamFile;
#define KFILE_IMPL struct KExtendableRamFile
#include <kfs/impl.h>

/*-----------------------------------------------------------------------
 * KExtendableRamFile
 *  A KFile that puts reads and writes into a buffer.
 *  The buffer can be realloced if necessary.
 */
struct KExtendableRamFile {
    KFile	dad;

    uint64_t pos;
    size_t   bsize;
    char *   buffer;
};

/* ----------------------------------------------------------------------
 * Destroy
 *
 */
static
rc_t CC KExtendableRamFileDestroy (KExtendableRamFile *self)
{
    free(self->buffer);
    memset(self, 0, sizeof *self);
    free(self);
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

static struct KSysFile *CC KExtendableRamFileGetSysFile(
    const KExtendableRamFile *self, uint64_t *offset)
{
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
static rc_t CC KExtendableRamFileRandomAccess (const KExtendableRamFile *self) {
    return 0;
}

/* ----------------------------------------------------------------------
 * Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KExtendableRamFileType (const KExtendableRamFile *self)
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
rc_t CC KExtendableRamFileSize (const KExtendableRamFile *self, uint64_t *size)
{
    assert (self != NULL);
    assert (size != NULL);

    *size = self->pos;
    return 0;
}

/* ----------------------------------------------------------------------
 * SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KExtendableRamFileSetSize(KExtendableRamFile *self, uint64_t size)
{
    return RC(rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported);
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
static rc_t CC KExtendableRamFileRead(const KExtendableRamFile *self,
    uint64_t pos, void *buffer, size_t bsize, size_t *num_read)
{
    assert (self);
    assert (buffer);
    assert (num_read);
    /* no restrictions on pos or bsize */

    if (pos >= self->pos)
    {
        *num_read = 0;
        return 0;
    }

    if (pos + bsize > self->pos)
        bsize = self->pos - pos;

    memmove(buffer, self->buffer + pos, bsize);

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
static rc_t CC KExtendableRamFileWrite(KExtendableRamFile *self,
    uint64_t pos, const void *buffer, size_t bsize, size_t *num_writ)
{
    assert (self);
    assert (buffer);
    assert (num_writ);

    /* if buffer big enough? */
    if (self->pos + bsize > self->bsize)
    {
        char *tmp = NULL;
        size_t req = self->bsize;
        while (req < self->pos + bsize) {
            req += self->bsize;
        }
        tmp = realloc(self->buffer, req);
        if (tmp == NULL) {
            return RC(rcFS, rcFile, rcWriting, rcMemory, rcExhausted);
        }
        self->buffer = tmp;
        self->bsize = req;
    }

    memmove(self->buffer + self->pos, buffer, bsize);
    self->pos += bsize;
    *num_writ = bsize;

    return 0;
}

static const KFile_vt_v1 vtKExtendableRamFile =
{
    /* version */
    1, 1,

    /* 1.0 */
    KExtendableRamFileDestroy,
    KExtendableRamFileGetSysFile,
    KExtendableRamFileRandomAccess,
    KExtendableRamFileSize,
    KExtendableRamFileSetSize,
    KExtendableRamFileRead,
    KExtendableRamFileWrite,

    /* 1.1 */
    KExtendableRamFileType
};

/* ----------------------------------------------------------------------
 * KExtendableRamFileMake
 *  create a new file object
 */
LIB_EXPORT rc_t CC KExtendableRamFileMake(KFile **pself) {
    size_t buffer_size = 1; /* TODO: set it to something good */
    rc_t rc = 0;
    KExtendableRamFile *self = NULL;

    if (pself == NULL)
        return RC(rcFS, rcFile, rcConstructing, rcSelf, rcNull);

    if (buffer_size == 0)
        ++buffer_size;

    /* ----- get space for the object */
    self = calloc(1, sizeof *self);
    if (self == NULL) {
        return RC(rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
    }
    self->buffer = calloc(1, buffer_size);
    if (self->buffer == NULL) {
        free(self);
        return RC(rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
    }
    rc = KFileInit(&self->dad,			/* initialize base class */
        (const KFile_vt*)&vtKExtendableRamFile,/*VTable for KExtendableRamFile*/
        "KExtendableRamFile", "ExtendableRAM", true, true);
    if (rc != 0) {
        free(self->buffer);
        free(self);
        return rc;
    }
    self->bsize = buffer_size;
    *pself = &self->dad;
    return 0;
}
