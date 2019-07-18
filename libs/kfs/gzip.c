/*==============================================================================
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

struct KGZipFile;
#define KFILE_IMPL struct KGZipFile

#include <kfs/extern.h>
#include <kfs/impl.h>  /* KFile_vt_v1 */
#include <kfs/gzip.h>  /* KFileMakeGzipFor... */
#include <klib/debug.h>
#include <klib/status.h>
#include <klib/rc.h>
#include <klib/out.h>
#include <sysalloc.h>

#include <zlib.h>      /* z_stream */
#include <assert.h>
#include <stdlib.h>    /* malloc */
#include <string.h> /* memset */

#ifdef _DEBUGGING
#define GZIP_DEBUG(msg) DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_GZIP), msg)
#else
#define GZIP_DEBUG(msg)
#endif

#if _DEBUGGING && 0
/* set limit to 1MB */
#define USE_FILE_LIMIT 0x100000
#endif

#ifdef USE_FILE_LIMIT
#include <kfs/limitfile.h>
#endif

/***************************************************************************************/
/* Gzip File                                                                     */
/***************************************************************************************/

#define GZFCHUNK 0x20000    /* 128K */
/** Gzip KFile structure */
struct KGZipFile {
    KFile dad;
    KFile *file; /* inderlying KFile */
    uint64_t filePosition;
    uint64_t myPosition;
    z_stream strm;
    unsigned char buff[GZFCHUNK]; /* buffer to cache KFile data */
    bool completed;
};
typedef struct KGZipFile KGZipFile;

/* virtual functions declarations (definitions for unsupported ) ***********************/

static struct KSysFile *CC s_GetSysFile(const KGZipFile *self,
    uint64_t *offset)
{ return NULL; }

static rc_t CC s_FileRandomAccess(const KGZipFile *self)
{ return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported ); }

static uint32_t CC s_FileType ( const KGZipFile *self )
{ return KFileType ( self -> file ); }

static rc_t CC s_FileSize(const KGZipFile *self, uint64_t *size)
{ return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported ); }

static rc_t CC s_FileSetSize(KGZipFile *self,
    uint64_t size)
{ return RC ( rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported ); }

/* read-only methods *******************************************************************/

static rc_t CC KGZipFile_InDestroy(KGZipFile *self);

static rc_t CC KGZipFile_InRead(const KGZipFile *cself,
    uint64_t pos,
    void *buffer,
    size_t bsize,
    size_t *num_read);

static rc_t CC KGZipFile_InWrite(KGZipFile *self,
    uint64_t pos,
    const void *buffer,
    size_t size,
    size_t *num_writ)
{ return RC ( rcFS, rcFile, rcWriting, rcFunction, rcUnsupported ); }

/** virtual table **********************************************************************/
static KFile_vt_v1 s_vtKFile_InGz = {
    /* version */
    1, 1,

    /* 1.0 */
    KGZipFile_InDestroy,
    s_GetSysFile,
    s_FileRandomAccess,
    s_FileSize,
    s_FileSetSize,
    KGZipFile_InRead,
    KGZipFile_InWrite,

    /* 1.1 */
    s_FileType
};

#define WINDOW_BITS (15 + 16)

/** Factory method definition **********************************************************/

LIB_EXPORT rc_t CC KFileMakeGzipForRead( const struct KFile **result,
    const struct KFile *file )
{
    rc_t rc;
    z_stream* strm;
    KGZipFile *obj;

    if ( result == NULL || file == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );

    obj = (KGZipFile*) malloc(sizeof(KGZipFile));
    if (!obj)
        return RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );

    rc = KFileInit(&obj->dad, (const KFile_vt*) &s_vtKFile_InGz, "KGZipFile", "no-name", true, false);
    if (rc != 0) {
        free(obj);
        return rc;
    }

    strm = &obj->strm;
    strm->zalloc   = Z_NULL;
    strm->zfree    = Z_NULL;
    strm->opaque   = Z_NULL;
    strm->avail_in = 0;
    strm->next_in  = Z_NULL;

    /* TBD - this should check gzlib error codes */
    if (inflateInit2(strm, WINDOW_BITS) != Z_OK) {
        free(obj);
        return RC ( rcFS, rcFile, rcConstructing, rcNoObj, rcUnknown );
    }

        obj->myPosition   = 0;
        obj->filePosition = 0;

    rc = KFileAddRef(file);
    if ( rc != 0 )
    {
        obj->file = NULL;
        KGZipFile_InDestroy ( obj );
    }
    else
    {
        obj->file = (KFile*) file;
        obj->completed = true; /* we could have an empty file and this would be okay */
        *result = &obj->dad;
    }

    return rc;
}

/* private functions declarations ******************************************************/

static rc_t z_read ( KGZipFile * self, void * buffer, size_t bsize, size_t * num_read );
static rc_t z_skip (KGZipFile *self, uint64_t pos);

/* virtual functions definitions *******************************************************/

static rc_t CC KGZipFile_InDestroy(KGZipFile *self) {
    rc_t rc = KFileRelease(self->file);
    if (rc == 0) {
        inflateEnd(&self->strm);
        free(self);
    }

    return rc;
}

static rc_t CC KGZipFile_InRead(const KGZipFile *cself,
    uint64_t pos,
    void *buffer,
    size_t bsize,
    size_t *num_read)
{
    KGZipFile *self = (KGZipFile*) cself;
    rc_t rc = 0;

    size_t numRead = 0, ignore;
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

    GZIP_DEBUG(("%s: pos %lu bsize %zu\n", __func__, pos, bsize));

    if (pos > self->myPosition)
    {
	rc =  z_skip (self, pos);
	if (rc)
	    return rc;
	if (pos != self->myPosition)
	    return 0;
    }

    rc = z_read ( self, buffer, bsize, &numRead );
    if (rc)
	return rc;

    *num_read = numRead;

    self->myPosition += numRead;

    return 0;
}

/* private functions definitions *******************************************************/

static rc_t z_read ( KGZipFile * self, void * buffer, size_t bsize, size_t * _num_read )
{
    rc_t rc = 0;
    size_t num_read, bleft ;

    GZIP_DEBUG(("%s: Enter\n",__func__));


    for (num_read = 0, bleft=bsize; bleft  > 0; )
    {
        z_stream * strm = &self->strm;
        size_t src_read;
        int zret;
        
        strm->next_out  = (uint8_t*)buffer + num_read;
        strm->avail_out = (uInt) bleft;

        GZIP_DEBUG(("%s: call inflate\n",__func__));


        GZIP_DEBUG(("%s: before inflate next_in %14p avail_in %6u total_in %10lu next_out %14p avail_out %6u total_out %10lu\n",
                    __func__, strm->next_in, strm->avail_in, strm->total_in,
                    strm->next_out, strm->avail_out, strm->total_out));

        zret = inflate (strm, Z_NO_FLUSH);

        GZIP_DEBUG(("%s: after inflate  next_in %14p avail_in %6u total_in %10lu next_out %14p avail_out %6u total_out %10lu\n",
                    __func__, strm->next_in, strm->avail_in, strm->total_in,
                    strm->next_out, strm->avail_out, strm->total_out));

	bleft = strm->avail_out;
	num_read = bsize - bleft;

        switch (zret)
        {
            /* unexpected error returns from zlib */
        default:
        case Z_ERRNO:
            GZIP_DEBUG(("%s: unknown error %d\n",__func__, zret));
            return RC (rcFS, rcFile, rcReading, rcFile, rcUnknown);

            /* known unfixable errors */
        case Z_STREAM_ERROR:
            GZIP_DEBUG(("%s: stream error %d\n",__func__, zret));
            return RC (rcFS, rcFile, rcReading, rcSelf, rcCorrupt);

        case Z_DATA_ERROR:
            GZIP_DEBUG(("%s: data error %d\n",__func__, zret));
            return RC (rcFS, rcFile, rcReading, rcData, rcCorrupt);

        case Z_STREAM_END:
            GZIP_DEBUG(("%s: stream end %d\n",__func__, zret));
            self->completed = true;
            zret = inflateReset (strm);
            GZIP_DEBUG (("%s: recall inflateReset zret = %d\n",__func__,zret));
            switch (zret)
            {
            case Z_OK:
                break;
            default:
                rc = RC (rcFS, rcFile, rcReading, rcData, rcInvalid);
                break;
            }
            break;

        case Z_BUF_ERROR:
            GZIP_DEBUG(("%s: buf error %d\n",__func__, zret));
            if (strm->avail_out > 0)
            {
                rc = KFileRead (self->file, self->filePosition, 
                                self->buff, sizeof (self->buff), &src_read);
                if (rc)
                    break;

                strm->avail_in = (uInt) src_read;
                self->filePosition += src_read;
                strm->next_in = self->buff;
                if (src_read)
                    self->completed = false;
                else if (self->completed)
                    goto done;
                else if (zret == Z_BUF_ERROR)
                {                
                    /* this is either a truncated file or a blocked stream
                     * code outside of here has to handle it */
                    GZIP_DEBUG(("%s: truncated input\n",__func__));
                    rc = RC (rcFS, rcFile, rcReading, rcData, rcInsufficient);
                    break;
                }
            }
            break;
        case Z_OK:
            break;
        }
        if (rc)
            break;
    }
done:
    *_num_read = num_read;
    return rc;
}

static rc_t z_skip (KGZipFile *self, uint64_t pos)
{
    rc_t rc = 0;
    size_t num_read = 0; /* superfluous to quiet compiler */
    size_t to_read;
    uint8_t buff [ 32 * 1024 ];

    GZIP_DEBUG(("%s: enter pos %lu\n",__func__, pos));

    for ( to_read = sizeof buff; self -> myPosition < pos; self -> myPosition += num_read )
    {
	if (self->myPosition + sizeof buff > pos)
	    to_read = pos - self->myPosition;

        GZIP_DEBUG(("%s: call z_read to_read %zu \n",__func__));

	rc = z_read ( self, buff, to_read, &num_read );

        GZIP_DEBUG(("%s: return z_read num_read %zu \n",__func__));

	if ( rc )
	    break;

	if (num_read == 0)
	    break;

    }
    return rc;
}

/***************************************************************************************/
/* Gzip Output File                                                                    */
/***************************************************************************************/

/* write-only methods ******************************************************************/

static rc_t CC KGZipFile_OutDestroy(KGZipFile *self);

static rc_t CC KGZipFile_OutRead(const KGZipFile *cself,
    uint64_t pos,
    void *buffer,
    size_t bsize,
    size_t *num_read)
{ return RC ( rcFS, rcFile, rcReading, rcFunction, rcUnsupported ); }

static rc_t CC KGZipFile_OutWrite(KGZipFile *self,
    uint64_t pos,
    const void *buffer,
    size_t bsize,
    size_t *num_writ);

/** virtual table **********************************************************************/
static KFile_vt_v1 s_vtKFile_OutGz = {
    /* version */
    1, 1,

    /* 1.0 */
    KGZipFile_OutDestroy,
    s_GetSysFile,
    s_FileRandomAccess,
    s_FileSize,
    s_FileSetSize,
    KGZipFile_OutRead,
    KGZipFile_OutWrite,

    /* 1.1 */
    s_FileType
};

/** Factory method definition **********************************************************/
LIB_EXPORT rc_t CC KFileMakeGzipForWrite( struct KFile **result,
    struct KFile *file )
{
    rc_t rc;

    if ( result == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( file == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
        else if ( ! file -> write_enabled )
        {
            if ( file -> read_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
            else
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        }
        else
        {
            KGZipFile * obj = calloc ( 1, sizeof * obj );
            if ( obj == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KFileInit( & obj->dad, (const KFile_vt*) &s_vtKFile_OutGz,
                    "KGZipFile", "no-name", false, true );
                if ( rc == 0 )
                {
                    /* The default value for the memLevel parameter is 8 */
                    if ( deflateInit2 (&obj->strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                             WINDOW_BITS, 8, Z_DEFAULT_STRATEGY) != Z_OK )
                    {
                        rc = RC ( rcFS, rcFile, rcConstructing, rcNoObj, rcUnknown );
                    }
                    else
                    {
#if USE_FILE_LIMIT
                        KFile * limitFile;
                        STATUS ( STAT_USR, "wrapping gzip output file in a limit file with limit %u bytes per block.\n", USE_FILE_LIMIT );
                        rc = KFileMakeLimitFile ( & limitFile, file, USE_FILE_LIMIT );
                        if ( rc == 0 )
                        {
                            obj -> file = limitFile;
                            * result = & obj -> dad;
                            return 0;
                        }
#else
                        rc = KFileAddRef ( file );
                        if ( rc == 0 )
                        {
                            obj -> file = file;
                            * result = & obj -> dad;
                            return 0;
                        }
#endif
                    }
                }

                free ( obj );
            }
        }

        * result = NULL;
    }

    return rc;
}

/* private functions declarations ******************************************************/

static int s_GzipAndWrite(KGZipFile *self,
    int flush,
    size_t *num_writ,
    rc_t *rc);

/* virtual functions definitions *******************************************************/

static rc_t CC KGZipFile_OutDestroy( KGZipFile *self)
{
    rc_t rc;
    if ( !self->completed )
    {
        int ret;
        size_t wrtn;
        z_stream* strm = &self->strm;
        strm->avail_in = 0;
        strm->next_in = Z_NULL;

        ret = s_GzipAndWrite( self, Z_FINISH, &wrtn, &rc );
        if ( rc != 0 )
            return rc;

        assert( ret == Z_STREAM_END ); /* stream will be complete */
        
        deflateEnd( strm ); /* clean up */
        self->completed = true;
    }

    rc = KFileRelease( self->file );
    if ( rc == 0 )
        free( self );

    return rc;
}

static rc_t CC KGZipFile_OutWrite( struct KGZipFile *self,
    uint64_t pos,
    const void *buffer,
    size_t bsize,
    size_t *num_writ )
{
    int ret;
    rc_t rc;
    z_stream* strm;
    size_t ignore;
    if (!num_writ)
    {   num_writ = &ignore; }

    *num_writ = 0;

    if ( pos != self->myPosition )
        return RC ( rcFS, rcFile, rcWriting, rcParam, rcInvalid );

    strm = &self->strm;
    strm->next_in  = (Bytef*) buffer;
    strm->avail_in = (uInt) bsize;

    rc = 0;
    ret = s_GzipAndWrite( self, Z_NO_FLUSH, num_writ, &rc );
    if ( rc != 0 )
        return rc;
    assert( ret != Z_STREAM_END );  /* stream will be complete */

    self->myPosition += * num_writ;

    return 0;
}

/* private functions definitions *******************************************************/

static int s_GzipAndWrite ( KGZipFile *self,
    int flush,
    size_t *num_writ,
    rc_t *rc )
{
    z_stream *strm;
    uInt avail_in;
    int ret;

    assert( self && num_writ && rc );

    *num_writ = 0;
    strm = &self->strm;
    avail_in = strm->avail_in;
    ret = 0;
    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do {
        uint32_t have;
        size_t written;
        strm->avail_out = sizeof( self->buff );
        strm->next_out = self->buff;
        ret = deflate( strm, flush );    /* no bad return value */
/*
        OUTMSG (( "deflate: ret=%u | avail-out+%u\n", ret, strm->avail_out ));
*/
        assert( ret != Z_STREAM_ERROR );  /* state not clobbered */
        have = sizeof( self->buff ) - strm->avail_out;
        written = 0;
        *rc = KFileWriteAll( self->file, self->filePosition, self->buff, have, &written );
        /* this is wrong - Z_ERRNO would tell us to check errno for error
           but the error is in *rc */
        if ( *rc != 0 )
            return Z_ERRNO;
        self->filePosition += written;
        *num_writ = avail_in - strm->avail_in;
    } while ( strm->avail_out == 0 );

    assert( strm->avail_in == 0 );     /* all input will be used */
    return ret;
}

/* EOF */

#include <stdio.h> /* printf */
LIB_EXPORT rc_t CC KFileMakeGzip2ForRead( const struct KFile **result,
    const struct KFile *file )
{
    rc_t rc;
    z_stream* strm;
    KGZipFile *obj;

    if ( result == NULL || file == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );

    obj = (KGZipFile*) malloc(sizeof(KGZipFile));
    if (!obj)
        return RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );

printf("KFileMakeGzip2ForRead %d\n", __LINE__);
    rc = KFileInit(&obj->dad, (const KFile_vt*) &s_vtKFile_InGz, "KGZipFile", "no-name", true, false);
printf("KFileMakeGzip2ForRead %d\n", __LINE__);
    if (rc != 0) {
        free(obj);
        return rc;
    }

    strm = &obj->strm;
    memset(strm, 0, sizeof *strm);
    strm->zalloc   = Z_NULL;
    strm->zfree    = Z_NULL;
    strm->opaque   = Z_NULL;
    strm->avail_in = 0;
    strm->next_in  = Z_NULL;

    /* TBD - this should check gzlib error codes */
printf("KFileMakeGzip2ForRead %d\n", __LINE__);
    if (inflateInit2(strm, WINDOW_BITS) != Z_OK) {
        free(obj);
        return RC ( rcFS, rcFile, rcConstructing, rcNoObj, rcUnknown );
    }

printf("KFileMakeGzip2ForRead %d\n", __LINE__);
        obj->myPosition   = 0;
        obj->filePosition = 0;

printf("KFileMakeGzip2ForRead %d\n", __LINE__);
    rc = KFileAddRef(file);
    if ( rc != 0 )
    {
printf("KFileMakeGzip2ForRead %d\n", __LINE__);
        obj->file = NULL;
        KGZipFile_InDestroy ( obj );
    }
    else
    {
printf("KFileMakeGzip2ForRead %d\n", __LINE__);
        obj->file = (KFile*) file;
        obj->completed = true; /* we could have an empty file and this would be okay */
        *result = &obj->dad;
    }

printf("KFileMakeGzip2ForRead %d\n", __LINE__);
    return rc;
}
