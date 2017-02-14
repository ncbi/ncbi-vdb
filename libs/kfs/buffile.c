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

struct KBufFile;
#define KFILE_IMPL struct KBufFile

#include <kfs/extern.h>
#include <kfs/buffile.h>

#include <kfs/file.h>
#include <kfs/impl.h>
#include <kfs/pagefile.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>


rc_t KBufWriteFileMakeWrite ( KFile ** bp, KFile * original, size_t bsize );


/*-----------------------------------------------------------------------
 * KBufFile
 */
typedef struct KBufFile KBufFile;
struct KBufFile
{
    KFile dad;

    uint64_t max_write;

    KFile *f;
    KPageFile *pf;

    KPage *pg;
    size_t pgsize;
    uint32_t pgid;
};


#define KBufFileSerialAccess( self ) \
    ( ( const KFile* ) & ( self ) -> dad ) -> align [ 0 ]

#define KBufFileSetSerialAccess( self, val ) \
    ( self ) -> dad . align [ 0 ] = ( val )
    

static
rc_t CC KBufFileDestroy ( KBufFile *self )
{
    rc_t rc = KPageRelease ( self -> pg );
    if ( rc == 0 )
    {
        self -> pg = NULL;
        self -> pgid = 0;

        rc = KPageFileRelease ( self -> pf );
        if ( rc == 0 )
        {
            KFileRelease ( self -> f );
            free ( self );
        }
    }

    return rc;
}

static
rc_t CC KBufFileDestroyTrunc ( KBufFile *self )
{
    /* grab some information */
    KFile *f = self -> f;
    rc_t rc = KFileAddRef ( self -> f );
    uint64_t max_write = self -> max_write;

    if ( rc == 0 )
    {
        /* destroy self and write all pages */
        rc = KBufFileDestroy ( self );
        if ( rc == 0 )
            rc = KFileSetSize ( f, max_write );

        KFileRelease ( f );
    }
    return rc;
}

static
struct KSysFile* CC KBufFileGetSysFile ( const KBufFile *self, uint64_t *offset )
{
    * offset = 0;
    return NULL;
}

static
rc_t CC KBufFileRandomAccess ( const KBufFile *self )
{
    return KFileRandomAccess ( self -> f );
}

static
rc_t CC KBufFileSizeRead ( const KBufFile *self, uint64_t *size )
{
    uint64_t lsize, fsize;
    rc_t rc = KPageFileSize ( self -> pf, & lsize, & fsize, NULL );

    if ( rc != 0 )
        * size = 0;
    else
        * size = fsize;

    return rc;
}

static
rc_t CC KBufFileSize ( const KBufFile *self, uint64_t *size )
{
    uint64_t lsize, fsize;
    rc_t rc = KPageFileSize ( self -> pf, & lsize, & fsize, NULL );

    if ( rc != 0 )
        * size = 0;
    else if ( lsize < fsize )
        * size = fsize;
    else
        * size = lsize;

    return rc;
}

static
rc_t CC KBufFileSetSize ( KBufFile *self, uint64_t size )
{
    KPageRelease ( self -> pg );
    self -> pg = NULL;
    self -> pgid = 0;

    return KPageFileSetSize ( self -> pf, size );
}

static
rc_t CC KBufFileNoSetSize ( KBufFile *self, uint64_t size )
{
    return RC ( rcFS, rcFile, rcResizing, rcFunction, rcUnsupported );
}

static
rc_t CC KBufFileRead ( const KBufFile *cself, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read )
{
    KBufFile *self = ( KBufFile* ) cself;

    rc_t rc;
    uint8_t *dst = buffer;
    size_t total, partial;

    for ( rc = 0, total = 0; total < bsize; pos += partial, total += partial )
    {
        const uint8_t *src;
        size_t bytes, offset;

        /* bytes remaining to be read */
        size_t remain = bsize - total;

        /* user wants to access this page */
        uint32_t pgid = ( uint32_t ) ( pos / self -> pgsize ) + 1;

        /* check against cached page */
        if ( self -> pgid != pgid )
        {
            /* release previous page */
            KPageRelease ( self -> pg );

            /* get requested page */
            rc = KPageFilePosGet ( self -> pf, & self -> pg, pos );
            if ( rc != 0 )
            {
                if ( GetRCState ( rc ) == rcNotFound )
                    rc = 0;

                self -> pg = NULL;
                self -> pgid = 0;
                break;
            }
            self -> pgid = pgid;
        }

        /* access page memory */
        rc = KPageAccessRead ( self -> pg, ( const void** ) & src, & bytes );
        if ( rc != 0 )
            break;

        /* going to copy from page at the given offset */
        offset = ( size_t ) ( pos % bytes );
        assert ( bytes == self -> pgsize );

        /* limit to end of file if serial access */
        if ( KBufFileSerialAccess ( self ) )
        {
            uint64_t fsize;
            rc = KPageFileSize ( self -> pf, NULL, & fsize, NULL );
            if ( rc != 0 )
                break;
            if ( pgid * bytes > fsize )
            {
                bytes = ( size_t ) ( fsize % bytes ); 
                if ( bytes <= offset )
                    break;
            }
        }
        /* limit to eof if read-only */
        else if ( ! self -> dad . write_enabled )
        {
            if ( pgid * bytes > self -> max_write )
            {
                bytes = ( size_t ) ( self -> max_write % bytes ); 
                if ( bytes <= offset )
                    break;
            }
        }

        /* limit bytes to request */
        partial = bytes - offset;
        if ( partial > remain )
            partial = remain;

        /* copy bytes */
        memmove ( & dst [ total ], & src [ offset ], partial );
    }

    if ( total != 0 )
    {
        * num_read = total;
        return 0;
    }

    * num_read = 0;
    return rc;
}

static
rc_t CC KBufFileWrite ( KBufFile *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ )
{
    rc_t rc;
    size_t total, partial;
    const uint8_t *src = buffer;

    /* record start marker */
    uint64_t start = pos;

    for ( rc = 0, total = 0; total < size; pos += partial, total += partial )
    {
        uint8_t *dst;
        size_t bytes, offset;

        /* bytes remaining to be writ */
        size_t remain = size - total;

        /* user wants to access this page */
        uint32_t pgid = ( uint32_t ) ( pos / self -> pgsize ) + 1;

        /* check against cached page */
        if ( self -> pgid != pgid )
        {
            /* release previous page */
            KPageRelease ( self -> pg );

            /* get requested page */
            rc = KPageFilePosGet ( self -> pf, & self -> pg, pos );
            if ( rc != 0 )
            {
                self -> pg = NULL;
                self -> pgid = 0;
                break;
            }
            self -> pgid = pgid;
        }

        /* access page memory */
        rc = KPageAccessUpdate ( self -> pg, ( void** ) & dst, & bytes );
        if ( rc != 0 )
            break;

        /* going to copy to page at the given offset */
        offset = ( size_t ) ( pos % bytes );
        partial = bytes - offset;
        assert ( bytes == self -> pgsize );

        /* limit bytes to request */
        if ( partial > remain )
            partial = remain;

        /* copy bytes */
        memmove ( & dst [ offset ], & src [ total ], partial );
    }

    if ( total != 0 )
    {
        if ( start + total > self -> max_write )
            self -> max_write = start + total;
        * num_writ = total;
        return 0;
    }

    * num_writ = 0;
    return rc;
}


static
rc_t CC KBufFileNoWrite ( KBufFile *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ )
{
    return RC ( rcFS, rcFile, rcWriting, rcFunction, rcUnsupported );
}

static
uint32_t CC KBufFileType ( const KBufFile * self )
{
    return KFileType ( self -> f );
}

static
const KFile_vt_v1 vtKBufFileRD_v1 =
{
    /* version */
    1, 1,

    /* 1.0 */
    KBufFileDestroy,
    KBufFileGetSysFile,
    KBufFileRandomAccess,
    KBufFileSizeRead,
    KBufFileNoSetSize,
    KBufFileRead,
    KBufFileNoWrite,

    /* 1.1 */
    KBufFileType
};

static
const KFile_vt_v1 vtKBufFileRW_v1 =
{
    /* version */
    1, 1,

    /* 1.0 */
    KBufFileDestroyTrunc,
    KBufFileGetSysFile,
    KBufFileRandomAccess,
    KBufFileSize,
    KBufFileSetSize,
    KBufFileRead,
    KBufFileWrite,

    /* 1.1 */
    KBufFileType
};

static
rc_t KBufFileMake ( KBufFile ** bp, const KFile_vt *vt,
    const KFile *f, uint64_t eof, const KPageFile *pf,
    bool read_enabled, bool write_enabled, bool serial )
{
    rc_t rc;

    KBufFile *buf = malloc ( sizeof * buf );
    if ( buf == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = KFileInit ( & buf -> dad, vt, "KBufFile", "no-name", read_enabled, write_enabled );
        if ( rc == 0 )
        {
            rc = KFileAddRef ( f );
            if ( rc == 0 )
            {
                buf -> max_write = serial ? 0 : eof;

                buf -> f = ( KFile* ) f;
                buf -> pf = ( KPageFile* ) pf;

                buf -> pg = NULL;
                buf -> pgsize = KPageConstSize ();
                buf -> pgid = 0;

                KBufFileSetSerialAccess ( buf, serial );

                * bp = buf;
                return 0;
            }
        }

        free ( buf );
    }

    return rc;
}

/* MakeBufferedRead
 *  make a read-only file buffer
 *
 *  "buf" [ OUT ] - return parameter for new buffered file
 *
 *  "original" [ IN ] - source file to be buffered. must have read access
 *
 *  "bsize" [ IN ] - buffer size
 */
LIB_EXPORT rc_t CC KBufFileMakeRead ( const KFile ** bp,
    const KFile * original, size_t bsize )
{
    rc_t rc;

    if ( bp == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( original == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else
        {
            uint64_t eof;
            bool serial = false;
            const KPageFile *pf;

            /* round bsize to next higher page */
            size_t pgmask = KPageConstSize () - 1;
            assert ( ( pgmask & ( pgmask + 1 ) ) == 0 );
            bsize = ( bsize + pgmask ) & ~ pgmask;

            /* check for serial access */
            if ( KFileRandomAccess ( original ) != 0 )
                serial = true;

            /* try to get end of file */
            rc = KFileSize ( original, & eof );
            if ( rc != 0 )
                serial = true;

            /* create page file for read */
            rc = KPageFileMakeRead ( & pf, original, bsize );
            if ( rc == 0 )
            {
                /* create buffered file */
                KBufFile *buf;
                rc = KBufFileMake ( & buf, ( const KFile_vt* ) & vtKBufFileRD_v1,
                    original, eof, pf, true, false, serial );
                if ( rc == 0 )
                {
                    * bp = & buf -> dad;
                    return 0;
                }

                KPageFileRelease ( pf );
            }
        }

        * bp = NULL;
    }

    return rc;
}


/* MakeBufferedWrite
 *  make a writable file buffer
 *
 *  "buf" [ OUT ] - return parameter for new buffered file
 *
 *  "original" [ IN ] - source file to be buffered. must have read access
 *
 *  "update" [ IN ] - if true, make a read/write buffer
 *
 *  "bsize" [ IN ] - buffer size
 */
LIB_EXPORT rc_t CC KBufFileMakeWrite ( KFile ** bp,
    KFile * original, bool update, size_t bsize )
{
    rc_t rc;

    if ( bp == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
    else
    {
        if ( original == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNull );
        else if ( ! original -> write_enabled )
        {
            if ( original -> read_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcReadonly );
            else
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcNoPerm );
        }
        else
        {
            /* check for serial access */
            bool serial = false;
            if ( KFileRandomAccess ( original ) != 0 )
                serial = true;

            /* if read-write is not required */
            if ( ! update )
                return KBufWriteFileMakeWrite ( bp, original, bsize );

            /* can only use page file if backing is r/w */
            if ( ! original -> read_enabled )
                rc = RC ( rcFS, rcFile, rcConstructing, rcFile, rcWriteonly );
            else
            {
                uint64_t eof;
                KPageFile *pf;

                /* round bsize to next higher page */
                size_t pgmask = KPageConstSize () - 1;
                assert ( ( pgmask & ( pgmask + 1 ) ) == 0 );
                bsize = ( bsize + pgmask ) & ~ pgmask;

                /* try to get end of file */
                rc = KFileSize ( original, & eof );
                if ( rc != 0 )
                    serial = true;

                /* create page file for update */
                rc = KPageFileMakeUpdate ( & pf, original, bsize, serial );
                if ( rc == 0 )
                {
                    /* create buffered file */
                    KBufFile *buf;
                    rc = KBufFileMake ( & buf, ( const KFile_vt* ) & vtKBufFileRW_v1,
                        original, eof, pf, update, true, serial );
                    if ( rc == 0 )
                    {
                        * bp = & buf -> dad;
                        return 0;
                    }

                    KPageFileRelease ( pf );
                }
            }
        }

        * bp = NULL;
    }

    return rc;
}
