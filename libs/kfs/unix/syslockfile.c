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

typedef struct KExclAccessFile KExclAccessFile;
#define KFILE_IMPL KExclAccessFile

#include <kfs/extern.h>
#include <kfs/lockfile.h>
#include <kfs/impl.h>
#include <kfs/file.h>
#include <kfs/directory.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <assert.h>

/*--------------------------------------------------------------------------
 * KExclAccessFile
 */
struct KExclAccessFile
{
    KFile dad;
    KLockFile *lock;
    KFile *f;
};

static
rc_t KExclAccessFileDestroy ( KExclAccessFile *self )
{
    rc_t rc = KFileRelease ( self -> f );
    if ( rc == 0 )
    {
        KLockFileRelease ( self -> lock );
        free ( self );
    }
    return rc;
}

static
struct KSysFile *KExclAccessFileGetSysFile ( const KExclAccessFile *self, uint64_t *offset )
{
    return KFileGetSysFile ( self -> f, offset );
}

static
rc_t KExclAccessFileRandomAccess ( const KExclAccessFile *self )
{
    return KFileRandomAccess ( self -> f );
}

static
rc_t KExclAccessFileSize ( const KExclAccessFile *self, uint64_t *size )
{
    return KFileSize ( self -> f, size );
}

static
rc_t KExclAccessFileSetSize ( KExclAccessFile *self, uint64_t size )
{
    return KFileSetSize ( self -> f, size );
}

static
rc_t KExclAccessFileRead ( const KExclAccessFile *self, uint64_t pos,
    void *buffer, size_t bsize, size_t *num_read )
{
    return KFileRead ( self -> f, pos, buffer, bsize, num_read );
}

static
rc_t KExclAccessFileWrite ( KExclAccessFile *self, uint64_t pos,
    const void *buffer, size_t size, size_t *num_writ )
{
    return KFileWrite ( self -> f, pos, buffer, size, num_writ );
}

static
uint32_t KExclAccessFileType ( const KExclAccessFile * self )
{
    return KFileType ( self -> f );
}

static KFile_vt_v1 KExclAccessFile_vt =
{
    1, 1,

    KExclAccessFileDestroy,
    KExclAccessFileGetSysFile,
    KExclAccessFileRandomAccess,
    KExclAccessFileSize,
    KExclAccessFileSetSize,
    KExclAccessFileRead,
    KExclAccessFileWrite,
    KExclAccessFileType
};

static
rc_t KExclAccessFileMake ( KExclAccessFile **fp, KLockFile *lock, KFile *f )
{
    rc_t rc;
    KExclAccessFile *xf = malloc ( sizeof * xf );
    if ( xf == NULL )
        rc = RC ( rcFS, rcFile, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = KFileInit ( & xf -> dad, ( const KFile_vt* ) & KExclAccessFile_vt,
            "KExclAccessFile", "no-name", f -> read_enabled, f -> write_enabled );
        if ( rc == 0 )
        {
            xf -> lock = lock;
            xf -> f = f;
            * fp = xf;
            return 0;
        }

        free ( xf );
    }

    return rc;
}


/*--------------------------------------------------------------------------
 * KDirectory
 *  interface extensions
 */


/* CreateExclusiveAccessFile
 *  opens a file with exclusive write access
 *
 *  "fp" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if true, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "access" [ IN ] - standard Unix access mode, e.g. 0664
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT rc_t CC KDirectoryVCreateExclusiveAccessFile ( KDirectory *self, KFile **fp,
    bool update, uint32_t access, KCreateMode mode, const char *path, va_list args )
{
    rc_t rc;

    if ( fp == NULL )
        rc = RC ( rcFS, rcFile, rcLocking, rcParam, rcNull );
    else
    {
        /* create the path */
        char full [ 4096 ];
        rc = KDirectoryVResolvePath ( self, true, full, sizeof full, path, args );
        if ( rc == 0 )
        {
            KLockFile *lock;

            /* acquire the lock */
            rc = KDirectoryCreateLockFile ( self, & lock, "%s.lock", full );
            if ( rc == 0 )
            {
                KFile *f;

                /* create the file */
                rc = KDirectoryCreateFile ( self, & f, update, access, mode, "%s", full );
                if ( rc == 0 )
                {
                    KExclAccessFile *xf;

                    /* wrap the file */
                    rc = KExclAccessFileMake ( & xf, lock, f );
                    if ( rc == 0 )
                    {
                        * fp = & xf -> dad;
                        return 0;
                    }

                    KFileRelease ( f );
                }

                KLockFileRelease ( lock );
            }
        }

        * fp = NULL;
    }

    return rc;
}
