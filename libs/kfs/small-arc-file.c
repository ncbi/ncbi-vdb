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
*
*/

typedef struct KSmallArcFile KSmallArcFile;
#define KFILE_IMPL KSmallArcFile
#include <kfs/file-impl.h>

#include "arc-files.h"
#include "toc-priv.h"

#include <kfs/file.h>
#include <klib/status.h>
#include <klib/rc.h>
#include <klib/data-buffer.h>
#include <kproc/timeout.h>
#include <kproc/lock.h>
#include <os-native.h>

/*--------------------------------------------------------------------------
 * KSmallArcFile
 *  a small file from an archive
 *  totally buffered in RAM
 */
struct KSmallArcFile
{
    KFile_v1 dad;

    uint64_t offset;
    const KFile_v1 * archive;
    KToc * toc;
    size_t start;
    size_t bytes;
};

static
rc_t CC KSmallArcFileWhack ( KSmallArcFile * self )
{
    KFileRelease ( self -> archive );
    KTocRelease ( self -> toc );
    free ( self );
    return 0;
}

static
struct KSysFile_v1 * CC KSmallArcFileGetSysFile ( const KSmallArcFile * self, uint64_t * offset )
{
    struct KSysFile_v1 * sys_file = KFileGetSysFile_v1 ( self -> archive, offset );
    if ( sys_file != NULL )
    {
        assert ( offset != NULL );
        * offset += self -> offset;
    }
    return sys_file;
}

static
rc_t CC KSmallArcFileRandomAccess ( const KSmallArcFile * self )
{
    return 0;
}

static
rc_t CC KSmallArcFileGetSize ( const KSmallArcFile * self, uint64_t *size )
{
    assert ( self != NULL );
    assert ( size != NULL );
    * size = self -> bytes;
    return 0;
}

static
rc_t CC KSmallArcFileSetSize ( KSmallArcFile * self, uint64_t size )
{
    return RC ( rcFS, rcArc, rcWriting, rcSelf, rcUnsupported );
}

static
rc_t KSmallArcFileFillTocBuffer ( const KSmallArcFile * self, timeout_t * tm )
{
    rc_t rc;
    KToc * toc = self -> toc;
    
    assert ( toc -> leading_file_end > toc -> leading_file_start );
    assert ( toc -> small_file_bytes >= ( toc -> leading_file_end - toc -> leading_file_start ) );
    assert ( toc -> small_file_data_lock != NULL );

    STATUS ( STAT_PRG, "acquiring TOC.small_file_data_lock" );
    rc = KLockAcquire ( toc -> small_file_data_lock );
    if ( rc == 0 )
    {
        STATUS ( STAT_GEEK, "re-testing for empty TOC.small_file_data" );
        if ( toc -> small_file_data . elem_count == 0 )
        {
            STATUS ( STAT_PRG, "creating TOC.small_file_data of size %lu", toc -> small_file_bytes );
            rc = KDataBufferMakeBytes ( ( KDataBuffer * ) & toc -> small_file_data, toc -> small_file_bytes );
            if ( rc == 0 )
            {
                size_t num_read = 0;
                size_t leading_bytes = ( size_t ) ( toc -> leading_file_end - toc -> leading_file_start );
                
                STATUS ( STAT_PRG, "reading %zu bytes of leading file data into TOC.small_file_data", leading_bytes );
                if ( self -> archive -> vt -> v1 . min < 2 )
                {
                    rc = KFileReadAll ( self -> archive, toc -> leading_file_start,
                        toc -> small_file_data . base, leading_bytes, & num_read );
                }
                else
                {
                    rc = KFileTimedReadAll ( self -> archive, toc -> leading_file_start,
                        toc -> small_file_data . base, leading_bytes, & num_read, tm );
                }
                STATUS ( STAT_PRG, "read %zu bytes of leading file data into TOC.small_file_data", num_read );

                STATUS ( STAT_GEEK, "setting TOC.small_file_bytes to %zu", num_read );
                toc -> small_file_bytes = num_read;
            }
        }

        if ( rc == 0 )
        {
            if ( toc -> small_file_bytes < ( uint64_t ) ( self -> offset + self -> bytes ) )
                rc = RC ( rcFS, rcArc, rcReading, rcTransfer, rcIncomplete );
        }
        
        STATUS ( STAT_PRG, "unlocking TOC.small_file_data_lock" );
        KLockUnlock ( toc -> small_file_data_lock );
    }
    return rc;
}

static
rc_t CC KSmallArcFileTimedRead ( const KSmallArcFile * self, uint64_t pos,
    void * buffer, size_t bsize, size_t * num_read, struct timeout_t * tm )
{
    STATUS ( STAT_PRG, "read request of %zu bytes from small archive file @ %lu", bsize, pos );
    if ( pos >= ( uint64_t ) self -> bytes )
    {
        STATUS ( STAT_PRG, "request was beyond EOF - no bytes read" );
        * num_read = 0;
    }
    else
    {
        const char * src;
        size_t offset = ( size_t ) pos + self -> start;

        size_t to_read = bsize;
        if ( ( size_t ) pos + bsize > self -> bytes )
            to_read = self -> bytes - ( size_t ) pos;

        STATUS ( STAT_PRG, "actual amount to read: %zu", to_read );

        STATUS ( STAT_GEEK, "checking for first read to small file buffer" );
        if ( self -> toc -> small_file_data . elem_count == 0 )
        {
            rc_t rc = KSmallArcFileFillTocBuffer ( self, tm );
            if ( rc != 0 )
            {
                STATUS ( STAT_PRG, "failed to read into small byte buffer" );
                if ( GetRCObject ( rc ) != ( enum RCObject ) rcTimeout || GetRCState ( rc ) != rcExhausted )
                    rc = KFileTimedRead ( self -> archive, self -> offset + pos, buffer, bsize, num_read, tm );
                return rc;
            }
        }

        src = self -> toc -> small_file_data . base;
        
        assert ( ( uint64_t ) ( offset + to_read ) <= self -> toc -> small_file_bytes );
        assert ( offset + to_read <= KDataBufferBytes ( & self -> toc -> small_file_data ) );

        STATUS ( STAT_GEEK, "copying %zu bytes into caller buffer", to_read );
        memmove ( buffer, & src [ offset ], to_read );

        STATUS ( STAT_PRG, "read %zu bytes of data from TOC.small_file_data", to_read );
        * num_read = to_read;
    }

    return 0;
}

static
rc_t CC KSmallArcFileRead ( const KSmallArcFile * self, uint64_t pos,
    void * buffer, size_t bsize, size_t * num_read )
{
    timeout_t tm;
    STATUS ( STAT_PRG, "creating 10 second timeout for read" );
    TimeoutInit ( & tm, 10 * 60 * 1000 );
    return KSmallArcFileTimedRead ( self, pos, buffer, bsize, num_read, & tm );
}

static
rc_t CC KSmallArcFileWrite ( KSmallArcFile * self, uint64_t pos,
    const void * buffer, size_t size, size_t * num_writ )
{
    assert ( num_writ != NULL );
    * num_writ = 0;
    return RC ( rcFS, rcArc, rcWriting, rcSelf, rcUnsupported );
}

static
uint32_t CC KSmallArcFileGetType ( const KSmallArcFile * self )
{
    return KFileType_v1 ( self -> archive );
}

static
rc_t CC KSmallArcFileTimedWrite ( KSmallArcFile * self, uint64_t pos,
    const void * buffer, size_t size, size_t * num_writ, struct timeout_t * tm )
{
    ( void ) tm;
    assert ( num_writ != NULL );
    * num_writ = 0;
    return RC ( rcFS, rcArc, rcWriting, rcSelf, rcUnsupported );
}


/* Make
 *  make a file that presents a window into the KToc small file buffer
 */
static KFile_vt_v1 vtKSmallArcFile =
{
    1, 2,

    KSmallArcFileWhack,
    KSmallArcFileGetSysFile,
    KSmallArcFileRandomAccess,
    KSmallArcFileGetSize,
    KSmallArcFileSetSize,
    KSmallArcFileRead,
    KSmallArcFileWrite,
    KSmallArcFileGetType,
    KSmallArcFileTimedRead,
    KSmallArcFileTimedWrite
};

rc_t KSmallArcFileMake ( const KFile_v1 ** fp, KToc * toc,
    const KFile_v1 * archive, uint64_t arc_loc, size_t boff, size_t bytes )
{
    rc_t rc = 0;

    if ( fp == NULL )
        rc = RC ( rcFS, rcArc, rcAllocating, rcParam, rcNull );
    else
    {
        if ( toc == NULL || archive == NULL )
            rc = RC ( rcFS, rcArc, rcAllocating, rcParam, rcNull );
        else
        {
            KSmallArcFile * f = malloc ( sizeof * f );
            if ( f == NULL )
                rc = RC ( rcFS, rcArc, rcAllocating, rcMemory, rcExhausted );
            else
            {
                rc = KFileInit_v1 ( & f -> dad, ( const KFile_vt * ) & vtKSmallArcFile, "KSmallArcFile", "", true, false );
                if ( rc == 0 )
                {
                    f -> offset = arc_loc;
                    f -> archive = archive;
                    f -> toc = toc;
                    f -> start = boff;
                    f -> bytes = bytes;

                    rc = KFileAddRef ( f -> archive );
                    if ( rc == 0 )
                    {
                        rc = KTocAddRef ( f -> toc );
                        if ( rc == 0 )
                        {
                            STATUS ( STAT_PRG, "created a KSmallArcFile @ %p", fp );
                            * fp = & f -> dad;
                            return 0;
                        }

                        KFileRelease ( f -> archive );
                    }
                }
                
                free ( f );
            }
        }
        
        * fp = NULL;
    }

    return rc;
}
