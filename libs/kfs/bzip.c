/*=======================================================================================
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
 *
 */

struct KBZipFile;
#define KFILE_IMPL struct KBZipFile

#include <kfs/extern.h>
#include <kfs/impl.h>
#include <kfs/bzip.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <klib/out.h>
#include <klib/log.h>
#include <sysalloc.h>

#include <bzlib.h>      /* bz_stream */
#include <assert.h>
#include <stdlib.h>    /* malloc */
#include <string.h>

#ifdef _DEBUGGING
#define BZIP_DEBUG(msg) DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_BZIP), msg)
#else
#define BZIP_DEBUG(msg)
#endif

#define BZIP_DBGSTREAM(strm,msg)                                 \
    BZIP_DEBUG(("%s: bz_stream (%p) %.35s\nnext_in %p "          \
                "avail_in %6u total_in %10lu next_out %p "       \
                "avail_out %6u total_out %10lu\n",               \
                __func__, strm, msg,                             \
                strm->next_in, strm->avail_in,                   \
                (uint64_t)strm->total_in_lo32 +                  \
                (((uint64_t)strm->total_in_hi32)<<32),           \
                strm->next_out, strm->avail_out,                 \
                (uint64_t)strm->total_out_lo32 +                 \
                (((uint64_t)strm->total_out_hi32)<<32)))


/***************************************************************************************/
/* bzip2 File                                                                    */
/***************************************************************************************/

#define BZ2CHUNK 0x20000    /* 128K */

/* ======================================================================
 * bzip2 KFile structure
 */
typedef struct KBZipFile KBZipFile;
struct KBZipFile
{
    KFile dad;
    KFile *file; /* underlying KFile */
    uint64_t filePosition;
    uint64_t myPosition;
    int BZ2_bzDecompressResult;
    bz_stream strm;
    char buff[BZ2CHUNK]; /* buffer to cache KFile data */
    bool completed;
};


/* ======================================================================
 * subroutine functions for KBZipFile methods
 */
static
rc_t KBZipFileWriteInt (KBZipFile *self,
                          int action,
                          size_t *pnumwrit)
{
    bz_stream *strm;
    unsigned avail_in;
    rc_t rc = 0;

    assert (self);
    assert (pnumwrit);

    *pnumwrit = 0;

    strm = &self->strm;
    avail_in = strm->avail_in;

    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do
    {
        uint32_t num_comp;
        size_t written;
        int zret;

        /* compress one internal buffers worth */
        strm->next_out = self->buff;
        strm->avail_out = sizeof (self->buff);

        zret = BZ2_bzCompress (strm, action);    /* no bad return value */

        /* state not clobbered */
        assert(zret == BZ_OK || zret == BZ_RUN_OK
               || zret == BZ_FINISH_OK || zret == BZ_STREAM_END);

        /* compression used the sizeof of the outbuffer - the amount
         * it says it didn't use */
        num_comp = sizeof(self->buff) - strm->avail_out;

        rc = KFileWrite (self->file, self->filePosition, self->buff, num_comp, &written);

        self->filePosition += written;

        *pnumwrit = avail_in - strm->avail_in;

    } while (strm->avail_out == 0);

    assert (strm->avail_in == 0);     /* all input will be used */
    return rc;
}


#if 0 /* obsolete */
static
int s_read(KBZipFile *self,
           char *buffer,
           size_t bsize,
           size_t *num_read,
           rc_t *rc)
{
    bz_stream* strm;
    int ret;

    assert(self && buffer && bsize && num_read);

    strm = &self->strm;

    ret = 0;
    while (!*num_read) {
        strm->next_out  = buffer;
        strm->avail_out = bsize;
        ret = BZ2_bzDecompress(strm);
        assert(ret == BZ_OK || ret == BZ_STREAM_END);  /* state not clobbered */
        *num_read = bsize - strm->avail_out;
        if (strm->avail_out > 0) {
            size_t src_read;
            * rc = KFileRead
                (self->file, self->filePosition, self->buff, sizeof(self->buff), &src_read);
            if (*rc != 0)
            {   return -70; }
            strm->avail_in = src_read;
            self->filePosition += src_read;
            strm->next_in = self->buff;
        }
        if (!strm->avail_in)
        {   break; }
    }
    return ret;
}

static rc_t old_KBZipFileReadInt ( KBZipFile * self, void * buffer, size_t bsize, size_t * num_read )
{
    rc_t rc = 0;
    size_t numRead = 0;

    do
    {
        int ret;
        size_t have = 0;

        if (self->BZ2_bzDecompressResult == BZ_STREAM_END)
            break;
        ret = s_read(self, (char*)buffer + numRead, bsize - numRead, &have, &rc);
        if ( ret == -70 ) /* rc hack - known to not collide with bzlib errors */
            return rc;
        self->BZ2_bzDecompressResult = ret;
        if (!have)
            break;
        numRead += have;
    } while (numRead != bsize);

    *num_read = numRead;
    return rc;
}
#endif

static
rc_t KBZipFileReadInt (KBZipFile * self, void * buffer, size_t bsize, size_t * pnumread)
{
    bz_stream temp;     /* store some values here during a reinit after stream end */
    bz_stream * strm;   /* alias for the object's bzip stream object */
    /* size_t bleft = bsize; */
    size_t num_read;
    size_t tot_read = 0;
    rc_t rc = 0;

    BZIP_DEBUG (("---------------\n%s: Enter requesting bsize %lu\n", __func__, bsize));

    strm = &self->strm;

    for (tot_read = 0; tot_read < bsize ; )
    {
        char * this_out;
        size_t src_read;
        int zret;
        bool bin;
        bool bout;
        bool end;

        bin = (strm->avail_in != 0);

        BZIP_DEBUG (("%s: loop start tot_read %zu\n", __func__, tot_read));

        strm->next_out = this_out = (char*)buffer + tot_read;
        strm->avail_out = (unsigned int) (bsize - tot_read);

        BZIP_DEBUG(("%s: call Decompress\n", __func__));

        BZIP_DBGSTREAM (strm, "before BZ2_bzDecompress");

        zret = BZ2_bzDecompress(strm);

        BZIP_DBGSTREAM (strm, "after BZ2_bzDecompress");

        switch (zret)
        {
            /* unexpected error returns from zlib */
        default:
            BZIP_DEBUG (("%s: undocumented error return in bzip Decompress\n", __func__));
            rc = RC (rcFS, rcFile, rcReading, rcFile, rcUnknown);
            PLOGERR (klogErr,
                     (klogErr, rc, "unknown error decompressing BZip2 file "
                      "error code '$(EC)'", "EC=%d", zret));
            return rc;

            /* known unfixable errors */
        case BZ_PARAM_ERROR:
            BZIP_DEBUG (("%s: internal programming error - bad parameters\n", __func__));
            rc = RC (rcFS, rcFile, rcReading, rcSelf, rcInvalid);
            if (strm == NULL)
                BZIP_DEBUG (("%s: strm is NULL\n", __func__));
            else
            {
                if (strm->state == NULL)
                    BZIP_DEBUG (("%s: strm->state is NULL\n", __func__));
                if (strm->avail_out < 1)
                    BZIP_DEBUG (("%s: strm->avail_out < 1\n", __func__));
            }
            LOGERR (klogInt, rc, "bzip strm structure bad");
            return rc;

        case BZ_DATA_ERROR:
            BZIP_DEBUG (("%s: data integrity error in bzip stream\n", __func__));
            rc = RC (rcFS, rcFile, rcReading, rcData, rcCorrupt);
            LOGERR (klogErr, rc, "bzip stream data error");
            return rc;

        case BZ_DATA_ERROR_MAGIC:
            BZIP_DEBUG (("%s: data magic bytes error in bzip stream\n", __func__));
            rc = RC (rcFS, rcFile, rcReading, rcData, rcIncorrect);
            LOGERR (klogErr, rc, "bzip stream not a bzip stream");
            return rc;

        case BZ_MEM_ERROR:
            BZIP_DEBUG (("%s: memory exhausted during BZip decompress\n", __func__));
            rc = RC (rcFS, rcFile, rcReading, rcMemory, rcExhausted);
            LOGERR (klogErr, rc, "not enough memory available during bzip decompress");
            return rc;

        case BZ_STREAM_END:
            BZIP_DEBUG (("%s: BZ_STREAM_END\n", __func__));
            end = true;
            num_read = strm->next_out - this_out;
            bout = (num_read != 0);
            BZIP_DEBUG (("%s: num_read %zu\n", __func__, num_read));
            self->completed = true;
            BZIP_DBGSTREAM (strm, "before BZ2_bzDecompressEnd");
            zret = BZ2_bzDecompressEnd(strm);
            BZIP_DBGSTREAM (strm, "after BZ2_bzDecompressEnd");
            if (zret == BZ_OK)
            {
                temp = *strm;
                memset (strm, 0, sizeof *strm);
                zret = BZ2_bzDecompressInit (strm, 1, 0);
                BZIP_DBGSTREAM (strm, "after BZ2_bzDecompressInit");
                strm->next_in = temp.next_in;
                strm->avail_in = temp.avail_in;
/*                 strm->next_out = temp.next_out; */
/*                 strm->avail_out = temp.avail_out; */
                strm->total_in_lo32 = temp.total_in_lo32;
                strm->total_in_hi32 = temp.total_in_hi32;
                strm->total_out_lo32 = temp.total_out_lo32;
                strm->total_out_hi32 = temp.total_out_hi32;
                BZIP_DBGSTREAM (strm, "after restore");
            }
            switch (zret)
            {
            case BZ_OK:
                break;
            default:
                return RC (rcFS, rcFile, rcReading, rcFile, rcUnknown);
            }
            goto read_more;

        case BZ_OK:
            BZIP_DEBUG (("%s: BZ_OK\n", __func__));
            end = false;
            num_read = strm->next_out - this_out;
            bout = (num_read != 0);
            BZIP_DEBUG (("%s: num_read %zu\n", __func__, num_read));

        read_more:
            /* if we wanted more on this read, read some more compressed */
            tot_read += num_read;
            if (strm->avail_in == 0)
            {
                rc = KFileRead (self->file, self->filePosition, self->buff, 
                                sizeof self->buff, &src_read);
                if (rc)
                    return rc;

                BZIP_DEBUG (("%s: KFileRead read %u\n", __func__, src_read));

                if (src_read == 0)
                {
                    BZIP_DEBUG (("%s: end %u in %u out %u\n", __func__, end, bin, bout));
                
                    if (!end && bin && !bout)
                        rc = RC (rcFS, rcFile, rcReading, rcData, rcInsufficient);
                    goto done;
                }
                strm->avail_in = (unsigned int) src_read;
                self->filePosition += src_read;
                strm->next_in = self->buff;

                /* if src_read == 0 but we are not at BZ_STREAM_END
                 * we have an error
                if (src_read == 0) bleft = 0;
                */
            }
            break;
        }
        if (rc)
            break;
        BZIP_DEBUG (("%s: loop end tot_read %zu\n", __func__, tot_read));
    }
done:
    BZIP_DEBUG (("%s: returning tot_read %zu\n\n\n", __func__, tot_read));
    *pnumread = tot_read;
    return rc;
}


static rc_t KBZipFileReadSeek (KBZipFile *self, uint64_t pos)
{
    rc_t rc = 0;
    size_t num_read = 0;
    size_t to_read;
    uint8_t buff [ 32 * 1024 ];

    for ( to_read = sizeof buff; self -> myPosition < pos; self -> myPosition += num_read )
    {
	if (self->myPosition + sizeof buff > pos)
	    to_read = pos - self->myPosition;

        BZIP_DEBUG(("%s: call KBZipFileReadInt to_read %zu \n",__func__));

	rc = KBZipFileReadInt ( self, buff, to_read, &num_read);

        BZIP_DEBUG(("%s: return KBZipFileReadInt num_read %zu \n",__func__));

	if ( rc )
	    break;

	if (num_read == 0)
	    break;
    }
    return rc;
}


/* ======================================================================
 * virtual table methods for KBZipFile object
 */

/* Destroy
 *  destroy file
 */
static rc_t CC KBZipFileDestroy (KBZipFile *self)
{
    rc_t rc = 0, orc = 0;

    if (self)
    {
        if (self->file != NULL)
        {
            int zret = BZ_OK;

            if (self->dad.write_enabled)
            {
                /* flush out end of compressed data */
/*                 if (self->completed == false) */
/*                 { */
                    size_t ignored;
                    bz_stream* strm = &self->strm;

                    strm->avail_in = 0;
                    strm->next_in = NULL;
                    rc = KBZipFileWriteInt(self, BZ_FINISH, &ignored);
/*                     assert (zret == BZ_STREAM_END); */        /* stream will be complete */
/*                 } */

                zret = BZ2_bzCompressEnd(&self->strm);   /* clean up */

                self->completed = true;
            }
            else if (self->dad.read_enabled)
            {
                zret = BZ2_bzDecompressEnd (&self->strm);
            }
            else
            {
                rc = RC (rcFS, rcFile, rcDestroying, rcSelf, rcInvalid);
                LOGERR (klogInt, orc, "corrupt object "
                        "closing bzip file object");
            }
            if (zret != BZ_OK)
            {
                orc = RC (rcFS, rcFile, rcDestroying, rcParam, rcInvalid);
                LOGERR (klogInt, orc, "bad parameters - coding error on "
                        "closing bzip file object");
                if (rc == 0)
                    rc = orc;
            }

            orc = KFileRelease (self->file);
            if (rc == 0)
                rc = orc;
        }
        free (self);
    }
    return rc;
}


/* GetSysFile
 *  returns an underlying system file object
 *  and starting offset to contiguous region
 *  suitable for memory mapping, or NULL if
 *  no such file is available.
 */
static struct KSysFile *CC KBZipFileGetSysFile (const KBZipFile *self,
                                                uint64_t *offset)
{
    return NULL;
}


/* RandomAccess
 *  ALMOST by definition, the file is random access
 *
 *  certain file types will refuse random access
 *  these include FIFO and socket based files, but also
 *  wrappers that require serial access ( e.g. compression )
 *
 *  returns 0 if random access, error code otherwise
 */
static
rc_t CC KBZipFileRandomAccess (const KBZipFile *self)
{
    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}


/* Size
 *  returns size in bytes of file
 *
 *  "size" [ OUT ] - return parameter for file size
 */
static
rc_t CC KBZipFileSize (const KBZipFile *self, uint64_t *size)
{
    return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported );
}


/* SetSize
 *  sets size in bytes of file
 *
 *  "size" [ IN ] - new file size
 */
static
rc_t CC KBZipFileSetSize(KBZipFile *self,
                                uint64_t size)
{
    return RC ( rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported );
}


/* Read
 *  read file from known position
 *
 *  "pos" [ IN ] - starting position within file
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - return buffer for read
 *
 *  "num_read" [ OUT ] - return parameter giving number of bytes
 *  actually read. when returned value is zero and return code is
 *  also zero, interpreted as end of file.
 */
static
rc_t CC KBZipFileRead (const KBZipFile *cself,
                              uint64_t pos,
                              void *buffer,
                              size_t bsize,
                              size_t *num_read)
{
    KBZipFile *self = (KBZipFile*) cself;
    rc_t rc = 0;

    size_t numRead = 0, ignore;

    if (self->dad.read_enabled == false)
        return RC (rcFS, rcFile, rcReading, rcFunction, rcUnsupported);

    if (!num_read)
    {   num_read = &ignore; }

    *num_read = 0;

    if (!cself || !buffer)
        return RC ( rcFS, rcFile, rcReading, rcParam, rcNull );

    if (!bsize)
    {   return 0; }

    if (pos < self->myPosition)
    {
        return RC ( rcFS, rcFile, rcReading, rcParam, rcInvalid );
    }

    BZIP_DEBUG(("%s: pos %lu bsize %zu\n", __func__, pos, bsize));
    BZIP_DEBUG(("%s: myPosition %lu numRead '%zu'\n", __func__, self->myPosition, numRead));

    if (pos > self->myPosition)
    {
        rc =  KBZipFileReadSeek (self, pos);
        if (rc)
            return rc;
        if (pos != self->myPosition)
            return 0;
    }

    rc = KBZipFileReadInt ( self, buffer, bsize, &numRead );
    if (rc)
        return rc;

    *num_read = numRead;

    self->myPosition += numRead;

    BZIP_DEBUG(("%s: myPosition %lu numRead '%zu'\n", __func__, self->myPosition, numRead));

    return 0;
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
rc_t CC KBZipFileWrite (struct KBZipFile *self,
                        uint64_t pos,
                        const void *buffer,
                        size_t bsize,
                        size_t *pnum_writ)
{
    rc_t rc = 0;

    if (self->dad.write_enabled == false)
        rc = RC (rcFS, rcFile, rcWriting, rcFunction, rcUnsupported);
    else
    {
        *pnum_writ = 0;

        if (pos != self->myPosition)
            rc = RC (rcFS, rcFile, rcWriting, rcParam, rcIncorrect);

        else
        {
            bz_stream * strm;
            size_t num_writ;

            strm = &self->strm;
            strm->next_in  = (char*) buffer;
            strm->avail_in = (unsigned int) bsize;

            rc =  KBZipFileWriteInt (self, BZ_RUN, &num_writ);
            if (rc == 0)
            {
                self->myPosition += num_writ;
                *pnum_writ = num_writ;
            }
        }
    }
    return rc;
}


/* Type
 *  returns a KFileDesc
 *  not intended to be a content type,
 *  but rather an implementation class
 */
static
uint32_t CC KBZipFileType (const KBZipFile *self)
{
    return KFileType (self->file);
}


/* ======================================================================
 * virtual table for KBZipFile object
 */
static
KFile_vt_v1 KBZipFile_vt_v1 =
{
    /* version */
    1, 1,

    /* 1.0 */
    KBZipFileDestroy,
    KBZipFileGetSysFile,
    KBZipFileRandomAccess,
    KBZipFileSize,
    KBZipFileSetSize,
    KBZipFileRead,
    KBZipFileWrite,

    /* 1.1 */
    KBZipFileType
};


/* ======================================================================
 * Factory functions
 */

LIB_EXPORT rc_t CC KFileMakeBzip2ForRead (const struct KFile **pnew_obj,
                                          const struct KFile *compfile)
{
    rc_t rc;

    if ( pnew_obj == NULL || compfile == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );

    else
    {
        KBZipFile *obj;

        *pnew_obj = NULL;

        obj = (KBZipFile*) calloc(1,sizeof(KBZipFile));
        if (!obj)
        {
            rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
            LOGERR (klogErr, rc, "memory exhausted building bzip2 "
                    "file object");
        }
        else
        {
            rc = KFileInit(&obj->dad, (const KFile_vt*) &KBZipFile_vt_v1,
                           "KBZipFile", "no-name", true, false);
            if (rc == 0)
            {
                bz_stream * strm;
                int zret;

                strm = &obj->strm;
                zret = BZ2_bzDecompressInit (strm, 1, /* verbosity */
                                             0); /* small */

                switch (zret)
                {
                case BZ_OK:
                    obj->completed = true;
                    rc = KFileAddRef (compfile);
                    if (rc == 0)
                    {
                        obj->file = (KFile *)compfile;
                        *pnew_obj = &obj->dad;
                        return 0;
                    }
                    break;

                case BZ_CONFIG_ERROR:
                    rc = RC (rcFS, rcFile, rcConstructing, rcLibrary,
                             rcCorrupt);
                    LOGERR (klogFatal, rc, "bzip2 library miscompiled");
                    break;

                case BZ_PARAM_ERROR:
                    rc = RC (rcFS, rcFile, rcConstructing, rcParam, rcInvalid);
                    LOGERR (klogInt, rc, "coding error bzip2 file object");
                    break;

                case BZ_MEM_ERROR:
                    rc = RC (rcFS, rcFile, rcConstructing, rcMemory,
                             rcExhausted);
                    LOGERR (klogErr, rc, "memory exhausted building bzip2 "
                            "file object");
                    break;

                default:
                    rc = RC (rcFS, rcFile, rcConstructing, rcLibrary,
                             rcUnexpected);
                    LOGERR (klogFatal, rc, "bzip2 library return unexpected "
                            "error");
                    break;

                }
            }
        }
        KBZipFileDestroy (obj);
    }
    return rc;
}


LIB_EXPORT rc_t CC KFileMakeBzip2ForWrite (struct KFile **pnew_obj,
                                           struct KFile *compfile)
{
    rc_t rc;

    if ( pnew_obj == NULL || compfile == NULL )
        rc= RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );

    else
    {
        KBZipFile *obj;

        *pnew_obj = NULL;

        obj = (KBZipFile*)calloc(1,sizeof(KBZipFile));
        if (obj == NULL)
        {
            rc = RC (rcFS, rcFile, rcConstructing, rcMemory, rcExhausted);
            LOGERR (klogErr, rc, "memory exhausted building bzip2 "
                    "file object");
        }
        else
        {
            rc = KFileInit(&obj->dad, (const KFile_vt*)&KBZipFile_vt_v1,
                           "KBZipFile", "no-name", false, true);
            if (rc == 0)
            {
                bz_stream* strm;
                int zret;

                strm = &obj->strm;
                zret = BZ2_bzCompressInit(strm, 9, /* blockSize100k */
                                          1, /* verbosity */
                                          30); /* workFactor */
                switch (zret)
                {
                case BZ_OK:
                    obj->completed = true;
                    rc = KFileAddRef (compfile);
                    if (rc == 0)
                    {
                        obj->file = compfile;
                        *pnew_obj = &obj->dad;
                        return 0;
                    }
                    break;

                case BZ_CONFIG_ERROR:
                    rc = RC (rcFS, rcFile, rcConstructing, rcLibrary,
                             rcCorrupt);
                    LOGERR (klogFatal, rc, "bzip2 library miscompiled");
                    break;

                case BZ_PARAM_ERROR:
                    rc = RC (rcFS, rcFile, rcConstructing, rcParam, rcInvalid);
                    LOGERR (klogInt, rc, "coding error bzip2 file object");
                    break;

                case BZ_MEM_ERROR:
                    rc = RC (rcFS, rcFile, rcConstructing, rcMemory,
                             rcExhausted);
                    LOGERR (klogErr, rc, "memory exhausted building bzip2 "
                            "file object");
                    break;

                default:
                    rc = RC (rcFS, rcFile, rcConstructing, rcLibrary,
                             rcUnexpected);
                    LOGERR (klogFatal, rc, "bzip2 library return unexpected "
                            "error");
                    break;

                }
            }
        }
        KBZipFileDestroy (obj);
    }
    return rc;
}


/* EOF */
