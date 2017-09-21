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

struct KLimitFile;
#define KFILE_IMPL struct KLimitFile

#include <kfs/extern.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <kfs/file.h>
#include <kfs/limitfile.h>
#include <sysalloc.h>

#include <assert.h>
#include <stdlib.h>

#include <kfs/impl.h>

/*-----------------------------------------------------------------------
 * KLimitFile
 */
typedef struct KLimitFile KLimitFile;
struct KLimitFile
{
    KFile dad;
    uint64_t block_size;
    KFile *	original;
};

/* Destroy
 */
static
rc_t CC KLimitFileDestroy (KLimitFile *self)
{
    rc_t rc = KFileRelease ( self -> original );
    if ( rc == 0 )
    {
        self -> original = NULL;
        free ( self );
    }
    return rc;
}

/* GetSysFile
 */

static
struct KSysFile * CC KLimitFileGetSysFile (const KLimitFile *self, uint64_t *offset)
{
    return KFileGetSysFile ( self -> original, offset );
}

/* RandomAccess
 */
static
rc_t CC KLimitFileRandomAccess (const KLimitFile *self)
{
    return KFileRandomAccess (self->original);
}

/* Size
 */
static
rc_t CC KLimitFileSize (const KLimitFile *self, uint64_t *size)
{
    return KFileSize ( self -> original, size );
}

/* SetSize
 */
static
rc_t CC KLimitFileSetSize (KLimitFile *self, uint64_t size)
{
    return KFileSetSize (self->original, size);
}

/* Read
 */
static
rc_t CC KLimitFileRead	(const KLimitFile *self,
				 uint64_t pos,
				 void *buffer,
				 size_t bsize,
				 size_t *num_read)
{
    uint64_t end = pos + bsize;
    uint64_t limit = ( pos + self -> block_size ) & ~ ( self -> block_size - 1 );
    if ( end > limit )
        end = limit;
    return KFileRead ( self -> original, pos, buffer, ( size_t ) ( end - pos ), num_read );
}

/* Write
 */
static
rc_t CC KLimitFileWrite (KLimitFile *self, uint64_t pos,
			   const void *buffer, size_t bsize,
			   size_t *num_writ)
{
    uint64_t end = pos + bsize;
    uint64_t limit = ( pos + self -> block_size ) & ~ ( self -> block_size - 1 );
    if ( end > limit )
        end = limit;
    return KFileWrite ( self -> original, pos, buffer, ( size_t ) ( end - pos ), num_writ );
}

/* Type
 */
static
uint32_t CC KLimitFileType (const KLimitFile *self)
{
    return KFileType (self->original);
}


static const KFile_vt_v1 vtKLimitFile =
{
    /* version */
    1, 1,

    /* 1.0 */
    KLimitFileDestroy,
    KLimitFileGetSysFile,
    KLimitFileRandomAccess,
    KLimitFileSize,
    KLimitFileSetSize,
    KLimitFileRead,
    KLimitFileWrite,

    /* 1.1 */
    KLimitFileType
};

/* ----------------------------------------------------------------------
 * KLimitFileMake
 *  create a new file object
 */

LIB_EXPORT rc_t CC KFileMakeLimitFile ( KFile ** pself, const KFile * original, size_t block_size )
{
    rc_t rc;

    if ( pself == NULL )
        rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
    else
    {
        if ( original == NULL )
            rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcNull );
        else if ( ( ( block_size - 1 ) & block_size ) != 0 )
            rc = RC ( rcFS, rcFile, rcAllocating, rcParam, rcInvalid ); 
        else
        {
            KLimitFile * obj = calloc ( 1, sizeof * obj );
            if ( obj == NULL )
                rc = RC ( rcFS, rcFile, rcAllocating, rcMemory, rcExhausted ); 
            else
            {
                rc = KFileInit ( & obj -> dad, (const KFile_vt*) & vtKLimitFile,
                                 "KLimitFile", "no-name",
                                 original->read_enabled,
                                 original->write_enabled
                    );
                if ( rc == 0 )
                {
                    rc = KFileAddRef ( original );
                    if ( rc == 0 )
                    {
                        obj -> original = ( KFile * ) original;
                        obj -> block_size = block_size;
                        * pself = & obj -> dad;
                        return 0;
                    }
                }

                free ( obj );
            }
        }

        * pself = NULL;
    }

    return rc;
}
