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

#define KFILE_IMPL struct KSparseFile_v1

#include <klib/log.h>
#include <klib/rc.h>
#include <kfs/extern.h>
#include <kfs/file.h>
#include <kfs/sparsefile.h>

#include <kfs/file-impl.h>

#include <sysalloc.h>

#include <assert.h>

struct KSparseFile_v1
{
    KFile_v1 dad;

    KFile_v1 *dense;
    
    uint64_t start;
    uint64_t block_size;
    uint64_t logical_size;

    /* going to need a block-id map */
};

static
rc_t CC KSparseFileDestroy_v1 ( KSparseFile_v1 *self )
{
    KFileRelease_v1 ( self -> dense );
    free ( self );
    return 0;
}

static
struct KSysFile_v1 *CC KSparseFileGetSysFile_v1 ( const KSparseFile_v1 *self, uint64_t *offset )
{
    return NULL;
}

static
rc_t CC KSparseFileRandomAccess_v1 ( const KSparseFile_v1 *self )
{
    return 0;
}

static
rc_t CC KSparseFileSize_v1 ( const KSparseFile_v1 *self, uint64_t *size )
{
    assert ( self != NULL );
    assert ( size != NULL );

    *size = self -> logical_size;

    return 0;
}

static
rc_t CC KSparseFileSetSize_v1 ( KSparseFile_v1 *self, uint64_t size )
{
    assert ( self != NULL );

    /* TBD - if the file is fixed size, this must fail with a non-zero rc */

    /* TBD - truncate the dense file if applicable */

    /* TBD - resize the block-id map */

    /* TBD - rewrite map and/or footer to dense file */

    self -> logical_size = size;
}

static
rc_t CC KSparseFileRead_v1 ( const KSparseFile_v1 *self, uint64_t pos, 
    void *buffer, size_t bsize, size_t *num_read )
{
    uint64_t req_end;

    /* TBD - if "pos" is >= logical_size, read NOTHING */

    /* TBD - calculate the end position, and clip against logical_size */
    req_end = pos + bsize;
    if ( req_end > self -> logical_size )
    {
        req_end = self -> logical_size;
        bsize = req_end - pos;
    }

    /* TBD - convert "pos" to starting 0-based block id */

    /* TBD - calculate the starting offset into physical block from "pos" */

    /* TBD - calculate the maximum available to read within the block */

    /* TBD - map starting block id to 1-based physical block id */

    /* TBD - if 1-based physical block id == 0, then block is virtual ( not present ).
       memset the buffer to fill with zeros and return */

    /* TBD - convert 1-based physical block id to 0-based */

    /* TBD - calculate start of physical block within dense file */

    /* return result of KFileRead on dense file, starting at physical block pos + offset, reading to read */

    return -1;
}

static
rc_t CC KSparseFileWrite_v1 ( KSparseFile_v1 *self, uint64_t pos, 
    const void *buffer, size_t bsize, size_t *num_writ )
{
    return -1;
}

static
uint32_t CC KSparseFileType_v1 ( const KSparseFile_v1 * self )
{
}

static
rc_t CC KSparseFileTimedRead_v1 ( const KSparseFile_v1 *self, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read, struct timeout_t *tm )
{
}

static
rc_t CC KSparseFileTimedWrite_v1 ( KSparseFile_v1 *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ, struct timeout_t *tm )
{
}

static const KFile_vt_v1 vtKSparseFile_v1 =
{
    /* version */
    1, 2,

    /* 1.0 */
    KSparseFileDestroy_v1,
    KSparseFileGetSysFile_v1,
    KSparseFileRandomAccess_v1,
    KSparseFileSize_v1,
    KSparseFileSetSize_v1,
    KSparseFileRead_v1,
    KSparseFileWrite_v1,

    /* 1.1 */
    KSparseFileType_v1,

    /* 1.2 */
    KSparseFileTimedRead_v1,
    KSparseFileTimedWrite_v1

};



/* AddRef
 * Release
 *  for KFile_v1
 *  resistant to NULL self references
 */
LIB_EXPORT rc_t CC KSparseFileAddRef_v1 ( const KSparseFile_v1 * self )
{
    return KFileAddRef_v1 ( & self -> dad );
}

LIB_EXPORT rc_t CC KSparseFileRelease_v1 ( const KSparseFile_v1 * self )
{
    return KFileRelease_v1 ( & self -> dad );
}


/* ToKFile
 *  cast the sparse file to a normal KFile
 *  returns a new reference to the KFile
 */
LIB_EXPORT rc_t CC KSparseFileToKFile_v1 ( const KSparseFile_v1 * self, KFile_v1 ** file )
{
    rc_t rc = 0;

    if ( file == NULL )
        rc = RC ();
    else
    {
        if ( self == NULL )
            rc = RC ();
        else
        {
            rc = KFileAddRef_v1 ( & self -> dad );
            if ( rc == 0 )
            {
                * file = ( KFile_v1 * ) & self -> dad;
                return 0;
            }
        }

        * file = NULL;
    }

    return rc;
}


/* Make
 *  make a sparse file from an existing KFile
 *  the existing file must be either empty or already a sparse file
 *
 *  "sparse" [ OUT ] - return parameter for sparse file
 *
 *  "dense" [ IN ] - backing file for dense data. must either be empty
 *   or a variable-size sparse file with the same block size
 *
 *  "block_size" [ IN ] - size of file blocks, e.g. 128 * 1024.
 *   must be an even power of 2, e.g. 0001 0010 0100 1000
 */
LIB_EXPORT rc_t CC KSparseFileMake_v1 ( KSparseFile_v1 ** sparse, struct KFile_v1 * dense, size_t block_size )
{
    rc_t rc;

    if ( sparse == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcSelf, rcNull );
    else
    {
        if ( dense == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );

        /* must have read capability */
        else if ( ! dense -> read_enabled )
            rc = RC (  );

        /* require block size to be non-zero, even power of two */
        else if ( block_size == 0 || ( block_size & ( block_size - 1 ) ) != 0 )
            rc = RC ();

        else
        {
            /* the dense file MUST be random-access */
            rc = KFileRandomAccess_v1 ( dense );
            if ( rc == 0 )
            {
                KSparseFile *spF;

                spF = calloc ( 1, sizeof * spF );
                if ( spF == NULL )
                    rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
                else
                {
                    rc = KFileInit_v1 ( &spF -> dad, ( const KFile_vt * ) &vtKSparseFile_v1, 
                        "KSparseFile", "no-name", true, dense -> write_enabled );
                    if ( rc == 0 )
                    {
                        rc = KFileAddRef ( dense );
                        if ( rc == 0 )
                        {
                            spF -> dense = dense;

                            spF -> block_size = block_size;
                            spF -> logical_size = 0;
                            spF -> start = 0;
                        
                            *sparse = spF;
                            return 0;
                        }
                    }

                    free ( spF );
                }
            }
        }

        * sparse = NULL;
    }

    return rc;
}

/* MakeFixed
 *  make a fixed-size sparse file from an existing KFile
 *  the existing file must be either empty or already a fixed-size sparse file of same size
 *
 *  "sparse" [ OUT ] - return parameter for sparse file
 *
 *  "dense" [ IN ] - backing file for dense data. must either be empty
 *   or a fixed-size sparse file with the same logical and block sizes
 *
 *  "block_size" [ IN ] - size of file blocks, e.g. 128 * 1024.
 *   must be an even power of 2.
 *
 *  "logical_size" [ IN ] - end of file in bytes.
 *   not required to be power of 2.
 */
LIB_EXPORT rc_t CC KSparseFileMakeFixed_v1 ( KSparseFile_v1 ** sparse, struct KFile_v1 * dense,
    size_t block_size, uint64_t logical_size );
{
    rc_t rc;

    if ( sparse == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcSelf, rcNull );
    else
    {
        if ( dense == NULL )
            rc = RC ( rcFS, rcFile, rcConstructing, rcParam, rcNull );
        /* require block size to be non-zero, even power of two */
        else if ( block_size == 0 || ( block_size & ( block_size - 1 ) ) != 0 )
            rc = RC ();
        else if ( logical_size == 0 )
            rc = RC ();
        else
        {
            KSparseFile *spF;

            spF = calloc ( 1, sizeof * spF );
            if ( spF == NULL )
                rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
            else
            {
                rc = KFileInit_v1 ( &spF -> dad, ( const KFile_vt * ) &vtKSparseFile_v1, 
                    "KSparseFile", "no-name", dense -> read_enabled, dense -> write_enabled );
                if ( rc == 0 )
                {
                    rc = KFileAddRef ( dense );
                    if ( rc == 0 )
                    {
                        spF -> dense = dense;

                        spF -> block_size = block_size;
                        spF -> logical_size = logical_size;
                        spF -> start = 0;
                        
                        *sparse = spF;
                        return 0;
                    }
                }

                free ( spF );
            }
        }

        * sparse = NULL;
    }

    return rc;
}
