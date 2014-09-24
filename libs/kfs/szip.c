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

struct KSZipFile;
#define KFILE_IMPL struct KSZipFile

#include <kfs/extern.h>
#include <kfs/impl.h>  /* KFile_vt_v1 */
#include <kfs/szip.h>  /* KFileMakeSzipFor... */
#include <klib/rc.h>
#include <sysalloc.h>


#include <ext/szlib.h>      /* sz_stream */

#include <assert.h>
#include <stdlib.h>    /* malloc */

#ifdef _DEBUGGING
#define SZIP_DEBUG(msg) DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_GZIP), msg)
#else
#define SZIP_DEBUG(msg)
#endif

/***************************************************************************************/
/* Szip File                                                                     */
/***************************************************************************************/

#define SZFCHUNK 0x20000    /* 128K */
/** Szip KFile structure */
struct KSZipFile {
    KFile dad;
    KFile *file; /* inderlying KFile */
    uint64_t filePosition;
    uint64_t myPosition;
    sz_stream strm;
    unsigned char buff[ SZFCHUNK ]; /* buffer to cache KFile data */
    bool completed;
};
typedef struct KSZipFile KSZipFile;

/* virtual functions declarations (definitions for unsupported ) ***********************/

static struct KSysFile *CC s_GetSysFile(const KSZipFile *self,
    uint64_t *offset)
{ return NULL; }

static rc_t CC s_FileRandomAccess(const KSZipFile *self)
{ return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported ); }

static uint32_t CC s_FileType ( const KSZipFile *self )
{ return KFileType ( self -> file ); }

static rc_t CC s_FileSize(const KSZipFile *self, uint64_t *size)
{ return RC ( rcFS, rcFile, rcAccessing, rcFunction, rcUnsupported ); }

static rc_t CC s_FileSetSize(KSZipFile *self,
    uint64_t size)
{ return RC ( rcFS, rcFile, rcUpdating, rcFunction, rcUnsupported ); }

/* read-only methods *******************************************************************/

static rc_t CC KSZipFile_InDestroy ( KSZipFile *self );

static rc_t CC KSZipFile_InRead ( const KSZipFile *cself,
    uint64_t pos,
    void *buffer,
    size_t bsize,
    size_t *num_read );

static rc_t CC KSZipFile_InWrite ( KSZipFile *self,
    uint64_t pos,
    const void *buffer,
    size_t size,
    size_t *num_writ )
{ return RC ( rcFS, rcFile, rcWriting, rcFunction, rcUnsupported ); }

/** virtual table **********************************************************************/
static KFile_vt_v1 s_vtKFile_InSz = {
    /* version */
    1, 1,

    /* 1.0 */
    KSZipFile_InDestroy,
    s_GetSysFile,
    s_FileRandomAccess,
    s_FileSize,
    s_FileSetSize,
    KSZipFile_InRead,
    KSZipFile_InWrite,

    /* 1.1 */
    s_FileType
};

#define WINDOW_BITS (15 + 16)

/** Factory method definition **********************************************************/

LIB_EXPORT rc_t CC KFileMakeSzipForRead( const struct KFile **result,
    const struct KFile *file )
{
    rc_t rc;
    sz_stream* strm;
    KSZipFile *obj;

    if ( result == NULL || file == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );

    obj = ( KSZipFile* ) malloc( sizeof( KSZipFile ) );
    if ( !obj )
        return RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );

    rc = KFileInit( &obj->dad, (const KFile_vt*) &s_vtKFile_InSz, true, false );
    if ( rc != 0 ) {
        free( obj );
        return rc;
    }

    strm = &obj->strm;
    strm->avail_in = 0;
    strm->next_in  = SZ_NULL;

    /* TBD - this should check gzlib error codes */
    if ( SZ_DecompressInit( strm ) != SZ_OK ) {
        free( obj );
        return RC ( rcFS, rcFile, rcConstructing, rcNoObj, rcUnknown );
    }

        obj->myPosition   = 0;
        obj->filePosition = 0;

    rc = KFileAddRef( file );
    if ( rc != 0 )
    {
        obj->file = NULL;
        KSZipFile_InDestroy ( obj );
    }
    else
    {
        obj->file = (KFile*) file;
        *result = &obj->dad;
    }

    return rc;
}

/* private functions declarations ******************************************************/

static rc_t s_read ( KSZipFile * self, void * buffer, size_t bsize, size_t * num_read );
static rc_t s_skip ( KSZipFile *self, uint64_t pos );

/* virtual functions definitions *******************************************************/

static rc_t CC KSZipFile_InDestroy( KSZipFile *self ) {
    rc_t rc = KFileRelease( self->file );
    if ( rc == 0 ) {
        SZ_DecompressEnd( &self->strm );
        free( self );
    }
    return rc;
}

static rc_t CC KSZipFile_InRead( const KSZipFile *cself,
    uint64_t pos,
    void *buffer,
    size_t bsize,
    size_t *num_read )
{
    KSZipFile *self = (KSZipFile*) cself;
    rc_t rc = 0;

    size_t numRead = 0, ignore;
    if ( num_read == NULL )
        num_read = &ignore;

    *num_read = 0;

    if ( !cself || !buffer )
        return RC ( rcFS, rcFile, rcReading, rcParam, rcNull );

    if ( !bsize )
        return 0;

    if ( pos < self->myPosition )
        return RC ( rcFS, rcFile, rcReading, rcParam, rcInvalid );

    SZIP_DEBUG(( "%s: pos %lu bsize %zu\n", __func__, pos, bsize ));

    if ( pos > self->myPosition )
    {
    rc =  s_skip ( self, pos );
    if ( rc != 0 )
        return rc;
    if ( pos != self->myPosition )
        return 0;
    }

    rc = s_read ( self, buffer, bsize, &numRead );
    if ( rc != 0 )
        return rc;

    *num_read = numRead;

    self->myPosition += numRead;

    return 0;
}

/* private functions definitions *******************************************************/

static rc_t s_read ( KSZipFile * self, void * buffer, size_t bsize, size_t * _num_read )
{
    rc_t rc = 0;
    size_t num_read = 0;

    SZIP_DEBUG(( "%s: Enter\n", __func__ ));

    for ( num_read = 0; num_read  == 0; )
    {
        sz_stream * strm = &self->strm;
        size_t src_read;
        int sret;
        
        strm->next_out = buffer;
        strm->next_out += num_read;
        strm->avail_out = bsize;

        SZIP_DEBUG (( "%s: call inflate\n", __func__ ));

        SZIP_DEBUG (( "%s: next_in %14p avail_in %6u total_in %10lu next_out %14p avail_out %6u total_out %10lu\n",
                     __func__, strm->next_in, strm->avail_in, strm->total_in,
                     strm->next_out, strm->avail_out, strm->total_out ));

        sret = SZ_Decompress ( strm, SZ_NO_FLUSH );

        SZIP_DEBUG (( "%s: next_in %14p avail_in %6u total_in %10lu next_out %14p avail_out %6u total_out %10lu\n",
                     __func__, strm->next_in, strm->avail_in, strm->total_in,
                     strm->next_out, strm->avail_out, strm->total_out ));

        num_read = bsize - strm->avail_out;

        switch ( sret )
        {
            /* unexpected error returns from slib */
        default:
        /* no equivalent in szlib.h
        case Z_ERRNO:
            SZIP_DEBUG (( "%s: unknown error %d\n",__func__, sret ));
            return RC ( rcFS, rcFile, rcReading, rcFile, rcUnknown );
        */

        /* known unfixable errors */
        case SZ_STREAM_ERROR:
            SZIP_DEBUG (( "%s: stream error %d\n",__func__, sret ));
            return RC ( rcFS, rcFile, rcReading, rcSelf, rcCorrupt );

            /* don't handle these yet */
        case SZ_STREAM_END:
            SZIP_DEBUG (( "%s: stream end %d\n", __func__, sret ));
            do
            {
                rc_t rc_2;
                uint8_t junkbuff[4096];

                rc_2 = KFileRead ( self->file, self->filePosition,
                                   junkbuff, sizeof (junkbuff), &src_read );
                if ( rc_2 != 0 )
                {
                    rc = rc_2;
                    break;
                }
                if ( ( ( rc == 0 ) && ( src_read != 0 ) ) || ( strm->avail_in > 0 ) )
                    rc = RC ( rcFS, rcFile, rcReading, rcData, rcExcessive );
            } while ( src_read );
            goto done;

            break;

        case SZ_MEM_ERROR:
            SZIP_DEBUG (( "%s: mem error %d\n", __func__, sret ));
            if ( strm->avail_out > 0 )
            {
                rc = KFileRead ( self->file, self->filePosition, 
                                 self->buff, sizeof (self->buff), &src_read );
                if ( rc != 0 )
                    break;

                strm->avail_in = src_read;
                self->filePosition += src_read;
                strm->next_in = (char*)self->buff;

                /* this is either a truncated file or a blocked stream
                 * code outside of here has to handle it */
                if ( ( src_read == 0 ) && ( sret == SZ_MEM_ERROR ) )
                {
                    SZIP_DEBUG (( "%s: truncated input\n", __func__ ));
                    rc = RC ( rcFS, rcFile, rcReading, rcData, rcInsufficient );
                    break;
                }

            }
            break;
        case SZ_OK:
            break;
        }
        if ( rc != 0 )
            break;
    }
done:
    *_num_read = num_read;
    return rc;
}


static rc_t s_skip ( KSZipFile *self, uint64_t pos )
{
    rc_t rc = 0;
    size_t num_read = 0; /* superfluous to quiet compiler */
    size_t to_read;
    uint8_t buff [ 32 * 1024 ];

    SZIP_DEBUG (( "%s: enter pos %lu\n", __func__, pos ));

    for ( to_read = sizeof buff; self -> myPosition < pos; self -> myPosition += num_read )
    {
    if ( self->myPosition + sizeof buff > pos )
        to_read = pos - self->myPosition;

    SZIP_DEBUG (( "%s: call z_read to_read %zu \n", __func__ ));

    rc = s_read ( self, buff, to_read, &num_read );

    SZIP_DEBUG (( "%s: return z_read num_read %zu \n", __func__ ));

    if ( rc != 0 )
        break;

    if ( num_read == 0 )
        break;
    }
    return rc;
}

/***************************************************************************************/
/* Szip Output File                                                                    */
/***************************************************************************************/

/* write-only methods ******************************************************************/

static rc_t CC KSZipFile_OutDestroy( KSZipFile *self );

static rc_t CC KSZipFile_OutRead( const KSZipFile *cself,
    uint64_t pos,
    void *buffer,
    size_t bsize,
    size_t *num_read )
{ return RC ( rcFS, rcFile, rcReading, rcFunction, rcUnsupported ); }

static rc_t CC KSZipFile_OutWrite( KSZipFile *self,
    uint64_t pos,
    const void *buffer,
    size_t bsize,
    size_t *num_writ );

/** virtual table **********************************************************************/
static KFile_vt_v1 s_vtKFile_OutSz = {
    /* version */
    1, 1,

    /* 1.0 */
    KSZipFile_OutDestroy,
    s_GetSysFile,
    s_FileRandomAccess,
    s_FileSize,
    s_FileSetSize,
    KSZipFile_OutRead,
    KSZipFile_OutWrite,

    /* 1.1 */
    s_FileType
};

/** Factory method definition **********************************************************/
LIB_EXPORT rc_t CC KFileMakeSzipForWrite( struct KFile **result,
    struct KFile *file )
{
    rc_t rc;
    sz_stream* strm;
    KSZipFile *obj;

    if ( result == NULL || file == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );

    obj = ( KSZipFile* ) malloc( sizeof( KSZipFile ) );
    if ( obj == NULL )
        return RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );

    rc = KFileInit( &obj->dad, (const KFile_vt*) &s_vtKFile_OutSz, false, true );
    if ( rc != 0 ) {
        free( obj );
        return rc;
    }

    strm = &obj->strm;
    strm->avail_in = 0;
    strm->next_in  = SZ_NULL;
    strm->total_out = 0;
    strm->next_out = 0;

    strm->hidden = 0;
    strm->options_mask = SZ_RAW_OPTION_MASK | SZ_NN_OPTION_MASK | SZ_MSB_OPTION_MASK;
    strm->bits_per_pixel = 8;
    strm->pixels_per_block = 8;
    strm->pixels_per_scanline = 16;
    strm->image_pixels = 16;

    /* TBD - this should check gzlib error codes */
    if ( SZ_CompressInit( strm ) != SZ_OK )
    {
        free( obj );
        return RC ( rcFS, rcFile, rcConstructing, rcNoObj, rcUnknown );
    }

    obj->myPosition   = 0;
    obj->filePosition = 0;
    obj->completed    = false;

    rc = KFileAddRef( file );
    if ( rc != 0 )
    {
        obj->file = NULL;
        KSZipFile_OutDestroy ( obj );
    }
    else
    {
        obj->file = file;
        *result = &obj->dad;
    }

    return rc;
}

/* private functions declarations ******************************************************/

static int s_SzipAndWrite ( KSZipFile *self,
    bool flush,
    size_t *num_writ,
    rc_t *rc );

/* virtual functions definitions *******************************************************/

static rc_t CC KSZipFile_OutDestroy( KSZipFile *self) {
    rc_t rc;
    if ( !self->completed ) {
        int ret;
        size_t wrtn;
        sz_stream* strm = &self->strm;
        strm->avail_in = 0;
        strm->next_in = SZ_NULL;

        ret = s_SzipAndWrite( self, SZ_FINISH, &wrtn, &rc );
        if ( rc != 0 )
            return rc;
        assert( ret == SZ_STREAM_END );        /* stream will be complete */
        
        SZ_CompressEnd( strm );   /* clean up */
            
        self->completed = true;
    }

    rc = KFileRelease( self->file );
    if ( rc == 0 )
        free( self );

    return rc;
}

static rc_t CC KSZipFile_OutWrite( struct KSZipFile *self,
    uint64_t pos,
    const void *buffer,
    size_t bsize,
    size_t *num_writ )
{
    int ret;
    rc_t rc;
    sz_stream* strm;
    size_t ignore;
    if ( num_writ == NULL )
        num_writ = &ignore;

    *num_writ = 0;

    if ( pos != self->myPosition )
        return RC ( rcFS, rcFile, rcWriting, rcParam, rcInvalid );

    strm = &self->strm;
    strm->next_in  = (char *) buffer;
    strm->avail_in = bsize;

    rc = 0;
    ret = s_SzipAndWrite( self, SZ_NO_FLUSH, num_writ, &rc );
    if ( rc != 0 )
        return rc;
    assert( ret != SZ_STREAM_END );        /* stream will be complete */

    self->myPosition += * num_writ;

    return 0;
}

/* private functions definitions *******************************************************/

static int s_SzipAndWrite( KSZipFile *self,
    bool flush,
    size_t *num_writ,
    rc_t *rc )
{
    sz_stream *strm;
    long avail_in;
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
        strm->avail_out = sizeof(self->buff);
        strm->next_out = self->buff;
        ret = SZ_Compress( strm, flush ? SZ_FINISH : SZ_NO_FLUSH );  /* no bad return value */
        assert( ret != SZ_STREAM_ERROR );  /* state not clobbered */
        have = sizeof( self->buff ) - strm->avail_out;
        written = 0;
        *rc = KFileWrite( self->file, self->filePosition, self->buff, have, &written );
        /* this is wrong - Z_ERRNO would tell us to check errno for error
           but the error is in *rc */
        if ( *rc != 0 )
            return SZ_STREAM_ERROR;
        self->filePosition += written;
        *num_writ = avail_in - strm->avail_in;
    } while ( strm->avail_out == 0 );
    assert( strm->avail_in == 0 );     /* all input will be used */
    return ret;
}

/* EOF */
