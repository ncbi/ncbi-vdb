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

#include <kdb/extern.h>
#include "wcolumn-priv.h"
#include "wcolidx2-priv.h"
#include "idxblk-priv.h"
#include "werror-priv.h"
#include <kfs/file.h>
#include <kfs/buffile.h>
#include <kfs/md5.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define IDX2_READ_FILE_BUFFER 1024 * 1024


/*--------------------------------------------------------------------------
 * KColumnIdx2
 *  level 2 index
 */

/* Init
 */
static
rc_t KColumnIdx2Init ( KColumnIdx2 *self, uint64_t idx2_eof )
{
    rc_t rc = KFileSize ( self -> f, & self -> eof );
    if ( rc == 0 )
    {
        if ( self -> eof < idx2_eof )
            rc = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcCorrupt );
        else
        {
            self -> eof = idx2_eof;
            return 0;
        }
    }

    KFileRelease ( self -> f );
    self -> f = NULL;
    return rc;
}

/* Create
 */
rc_t KColumnIdx2Create ( KColumnIdx2 *self,
    KDirectory *dir, KMD5SumFmt *md5, KCreateMode mode, uint64_t eof )
{
    rc_t rc = KColumnFileCreate (&self->f, &self->fmd5, dir, md5, mode, true, "idx2");
    if ( rc == 0 )
        rc = KColumnIdx2Init ( self, eof );
    return rc;
}

/* Open
 */
rc_t KColumnIdx2OpenRead ( KColumnIdx2 *self,
    const KDirectory *dir, uint64_t eof )
{
    rc_t rc = KDirectoryOpenFileRead ( dir,
        ( const KFile** ) & self -> f, "idx2" );
#if IDX2_READ_FILE_BUFFER
    if ( rc == 0 )
    {
        KFile * orig = self -> f;
        rc = KBufFileMakeRead ( ( const KFile** ) & self -> f, self -> f, eof + 1 );
	if ( rc == 0 )
        {
            KFileRelease ( orig );
        }
        else
        {
            self -> f = orig;
            rc = 0;
        }
    }
#endif
    if ( rc == 0 )
        rc = KColumnIdx2Init ( self, eof );
    return rc;
}

/* Whack
 */
rc_t KColumnIdx2Whack ( KColumnIdx2 *self )
{
    rc_t rc = KFileRelease ( self -> f );
    if ( rc == 0 )
    {
        self -> f = NULL;
        self -> fmd5 = NULL;
    }
    return rc;
}

/* LocateBlob
 *  locate an existing blob
 */
rc_t KColumnIdx2LocateBlob ( const KColumnIdx2 *self,
    KColBlobLoc *loc, const KColBlockLoc *bloc,
    int64_t first, int64_t upper, bool bswap )
{
    rc_t rc;

    /* compression not supported */
    if ( bloc -> u . blk . compressed )
        rc = RC ( rcDB, rcIndex, rcSelecting, rcNoObj, rcUnsupported );
    else
    {
        uint64_t buffer [ 1024 / 8 ]; /* make sure is uint64_t aligned */
        void *block = buffer;

        /* determine the number of entries in block */
        size_t orig = bloc -> u . blk . size;
        uint32_t count = KColBlockLocEntryCount ( bloc, & orig );

        /* determine the size to allocate */
        size_t block_size = KColBlockLocAllocSize ( bloc, orig, count );

        /* allocate a block */
        if ( block_size > sizeof buffer )
            block = malloc ( block_size );
        if ( block == NULL )
            rc = RC ( rcDB, rcIndex, rcSelecting, rcMemory, rcExhausted );
        else
        {
            size_t num_read;
            rc = KFileReadAll ( self -> f, bloc -> pg, block, orig, & num_read );
            if ( rc == 0 )
            {
                if ( num_read != orig )
                    rc = RC ( rcDB, rcIndex, rcSelecting, rcTransfer, rcIncomplete );
                else
                {
                    KColIdxBlock iblk;
                    rc = KColIdxBlockInit ( & iblk, bloc, orig, block, block_size, bswap );
                    if ( rc == 0 )
                    {
                        uint32_t span;
                        int64_t start_id;
                        int slot = KColIdxBlockFind ( & iblk,
                            bloc, count, first, & start_id, & span );
                        if ( slot < 0 )
                            rc = RC ( rcDB, rcIndex, rcSelecting, rcRange, rcNotFound );
                        else if ( upper > ( start_id + span ) )
                            rc = RC ( rcDB, rcIndex, rcSelecting, rcRange, rcInvalid );
                        else
                        {
                            loc -> start_id = start_id;
                            loc -> id_range = span;

                            KColIdxBlockGet ( & iblk,
                                bloc, count, slot, & loc -> pg, & span );
                            loc -> u . blob . size = span;
                        }
                    }
                }
            }

            if ( block != buffer )
                free ( block );
        }
    }

    return rc;
}

/* Write
 *  writes block to idx2 at end of file
 *
 *  "pos" [ OUT ] - location at which block was written
 *
 *  "buffer" [ IN ] and "bytes" [ IN ] - describes data buffer
 *
 *  return values:
 */
rc_t KColumnIdx2Write ( KColumnIdx2 *self,
    uint64_t *pos, const void *buffer, size_t bytes )
{
    rc_t rc;
    size_t num_writ;

    * pos = self -> eof;

    rc = KFileWrite ( self -> f,
        self -> eof, buffer, bytes, & num_writ );
    if ( rc == 0 )
    {
        if ( num_writ != bytes )
            rc = RC ( rcDB, rcIndex, rcWriting, rcTransfer, rcIncomplete );
    }

    return rc;
}

/* Commit
 *  keeps changes indicated by block size
 */
rc_t KColumnIdx2Commit ( KColumnIdx2 *self, size_t bytes )
{
    self -> eof += bytes;
    return 0;
}

rc_t KColumnIdx2CommitDone ( KColumnIdx2 *self )
{
    assert ( self != NULL );

    if ( self -> fmd5 != NULL )
        return KMD5FileCommit ( self -> fmd5 );

    return 0;
}
