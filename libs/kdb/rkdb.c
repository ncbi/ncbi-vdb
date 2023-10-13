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

#include <kdb/manager.h>

#include "rkdb.h"
#include "dbmgr.h"

#include <kfs/directory.h>

#include <klib/text.h>
#include <klib/rc.h>

#include <stdio.h>

#ifndef SUPPORT_VFS_URI
#define SUPPORT_VFS_URI 0
#endif

#ifndef SUPPORT_KDB_TAR
#define SUPPORT_KDB_TAR 0
#endif

/*--------------------------------------------------------------------------
 * KDB utility
 */

/* KDBHdrValidate
 *  validates that a header sports a supported byte order
 *  and that the version is within range
 */
/* this is included to make kdb.c vs. wkdb.c file comparisons easier in [x]emacs.*/
#if NOT_USED_IN_READ_ONLY_SIDE
rc_t KDBHdrValidate ( const KDBHdr *hdr, size_t size,
    uint32_t min_vers, uint32_t max_vers )
{
    assert ( hdr != NULL );

    if ( size < sizeof * hdr )
        return RC ( rcDB, rcHeader, rcValidating, rcData, rcCorrupt );

    if ( hdr -> endian != eByteOrderTag )
    {
        if ( hdr -> endian == eByteOrderReverse )
            return RC ( rcDB, rcHeader, rcValidating, rcByteOrder, rcIncorrect );
        return RC ( rcDB, rcHeader, rcValidating, rcData, rcCorrupt );
    }

    if ( hdr -> version < min_vers || hdr -> version > max_vers )
        return RC ( rcDB, rcHeader, rcValidating, rcHeader, rcBadVersion );

    return 0;
}
#endif


#if SUPPORT_VFS_URI
#else
/* return configured password as ASCIZ
 * opertates on vfs/kfs/kfg objects, not kdb objects */
static
rc_t KDBOpenFileGetPassword (char * pw, size_t pwz)
{
    VFSManager * mgr;
    rc_t rc;

    assert (pw);
    assert (pwz);

    pw[0] = '\0';

    rc = VFSManagerMake (&mgr);
    if (rc)
        ;                      /* failure to make VFS manager: pass along rc */
    else
    {
        size_t pwfz;
        char pwf [4096 + 1];

        rc = VFSManagerGetConfigPWFile (mgr, pwf, sizeof (pwf) - 1, &pwfz);
        if (rc)
            /* failure to get password file path: tweak rc */
            rc = RC (rcDB, rcMgr, rcOpening, rcEncryptionKey, rcNotFound);

        else
        {
            VPath * pwp;

            pwf [pwfz] = '\0'; /* force to ASCIZ */

#if 0
            rc = VPathMakeSysPath (&pwp, pwf);
#else
            rc = VFSManagerMakePath (mgr, &pwp, "%s", pwf);
#endif

            if (rc)
                ;       /* failure to construct a path from the string */

            else
            {
                const KFile * pwf;

                rc = VFSManagerOpenFileRead (mgr, &pwf, pwp);
                if (rc)
                    /* failure to open password file */
                    rc = RC (rcDB, rcMgr, rcOpening, rcEncryptionKey, rcNotOpen);

                else
                {
                    size_t z;
                    char pwb [4098]; /* arbitrarily using 4096 as maximum
                                        allowed length */

                    /* at this point we are only getting the password from a
                     * file but in the future if we can get it from a pipe of
                     * some sort we can't count on the ReadAll to really know
                     * if we hit end of file and not just a pause in the
                     * streaming.  VFS/KFS 2 will have to fix this somehow
                     */

                    rc = KFileReadAll (pwf, 0, pwb, sizeof pwb, &z);
                    if (rc)
                        ;       /* failure to read password file: pass along rc */
                    else
                    {
                        /* trim off EOL if present */
                        char * pc;

                        pwb[z] = '\0';   /* force ASCIZ */

                        pc = string_chr (pwb, z, '\r');
                        if (pc)
                        {
                            *pc = '\0';
                            z = 1 + pc - pwb;
                        }
                        pc = string_chr (pwb, z, '\n');
                        if (pc)
                        {
                            *pc = '\0';
                            z = 1 + pc - pwb;
                        }
                        if (z == 0)
                            rc = RC (rcDB, rcMgr, rcOpening, rcEncryptionKey, rcTooShort);

                        else if (pwz < z) /* pwz came in as 4096 */
                            rc = RC (rcDB, rcMgr, rcOpening, rcEncryptionKey, rcTooLong);

                        else
                        {
                            memmove (pw, pwb, z+1);
                        }
                    }
                    KFileRelease (pwf);
                }
                VPathRelease (pwp);
            }
        }
        VFSManagerRelease (mgr);
    }
    return rc;
}


/* not KDB specific - just uses vfs/krypto/kfs objects */
static
rc_t KDBOpenFileAsDirectory (const KDirectory * dir,
                             const char * path,
                             const KDirectory ** pdir,
                             uint32_t rcobj)
{
    const KFile * file;
    const KFile * f;
    const KDirectory * ldir;
    bool encrypted = false;

    rc_t rc;

    *pdir = NULL;

    rc = KDirectoryOpenFileRead (dir, &file, "%s", path);
    if (rc == 0)
    {
        rc = KFileRandomAccess(file);
        if (rc)
            rc = RC (rcDB, rcMgr, rcOpening, rcobj, rcUnsupported);
        else
        {
            size_t tz;
            char tbuff [4096];
            char pbuff [4096 + 1];

            rc = KFileReadAll (file, 0, tbuff, sizeof tbuff, &tz);
            if (rc == 0)
            {
                if (KFileIsEnc (tbuff, tz) == 0)
                {
                    encrypted = true;

                    rc = KDBOpenFileGetPassword (pbuff, sizeof (pbuff) - 1);
                    if (rc == 0)
                    {
                        KKey key;

                        rc = KKeyInitRead (&key, kkeyAES128, pbuff, string_size (pbuff));
                        if (rc == 0)
                        {
                            rc = KEncFileMakeRead (&f, file, &key);
                            if (rc == 0)
                            {
                                /* KEncFileMakeRead adds a reference */
                                KFileRelease (file);
                                file = f;
                                rc = KFileReadAll (file, 0, tbuff, sizeof tbuff, &tz);
                            }
                        }
                    }
                }
                else if (KFileIsWGAEnc (tbuff, tz) == 0)
                {
                    encrypted = true;

                    rc = KDBOpenFileGetPassword (pbuff, sizeof (pbuff) - 1);
                    if (rc == 0)
                    {
                        rc = KFileMakeWGAEncRead (&f, file, pbuff, string_size (pbuff));
                        if (rc == 0)
                        {
                            /* KFileMakeWGAEncRead adds a reference */
                            KFileRelease (file);
                            file = f;
                            rc = KFileReadAll (file, 0, tbuff, sizeof tbuff, &tz);
                        }
                    }
                }
                /* else not a handled encryption or unencrypted: we can't distinguish too much */

                if (rc == 0)
                {
                    if (KFileIsSRA (tbuff, tz) == 0)
                    {
                        rc = KDirectoryOpenSraArchiveReadUnbounded_silent_preopened (dir,
                                                                                     &ldir,
                                                                                     false,
                                                                                     file,
                                                                                     "%s",
                                                                                     path);
                    }
                    else
                    {
#if SUPPORT_KDB_TAR
                        rc = KDirectoryOpenTarArchiveRead_silent_preopened (dir, &ldir, false,
                                                                            file, "%s", path);
#else
                        /* will be reset immediately below */
                        rc = -1;
#endif
                    }

                    /* not an archive type we handle or a bad archive */
                    if ( rc != 0 )
                    {
                        if (encrypted)
                            rc = RC ( rcDB, rcMgr, rcOpening, rcEncryptionKey, rcIncorrect );
                        else
                            rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
                    }
                    else
                    {
                        /*
                         * release our ownership of the KFile that but archive will
                         * keep theirs
                         */
                        KFileRelease (file);
                        *pdir = ldir;
                        return 0;
                    }
                }
            }
        }
        KFileRelease (file);
    }
    return rc;
}
#endif

/* Writable
 *  examines a directory structure for any "lock" files
 */
rc_t KDBRWritable ( const KDirectory *dir, const char *path )
{
    uint32_t access;
    rc_t rc;

    /* protect us from bad parameters */
    if (dir == NULL)
        return RC (rcDB, rcPath, rcAccessing, rcDirectory, rcNull);
    if (path == NULL)
        return RC (rcDB, rcPath, rcAccessing, rcPath, rcNull);

    /* we have to be able to check the access if it is to be writable */
    rc = KDirectoryAccess ( dir, & access, "%s", path );
    if ( rc == 0 )
    {
        /* if there is a lock (or deprecated sealed) file in this directory */
        switch ( KDirectoryPathType ( dir, "%s/lock", path ) )
        {
        case kptFile:
        case kptFile | kptAlias:
            rc = RC ( rcDB, rcPath, rcAccessing, rcLock, rcLocked );
            break;
        case kptNotFound:
            /* much simpler handling for the sealed file */
            switch ( KDirectoryPathType ( dir, "%s/sealed", path ) )
            {
            case kptFile:
            case kptFile | kptAlias:
                rc = RC ( rcDB, rcPath, rcAccessing, rcLock, rcLocked );
                break;
            case kptNotFound:
                /* check if there are no write permissions */
                if ( ( access & 0222 ) == 0 )
                    rc = RC ( rcDB, rcPath, rcAccessing, rcPath, rcReadonly );
                /* else rc is still 0 from VAccess */
            }
            break;
        case kptBadPath:
            /* likely to be a non-driectory or something */
            rc = RC ( rcDB, rcPath, rcAccessing, rcPath, rcInvalid);
            break;
        default:
            /* an illegal type of object named "lock" is in this directory
             * which will block the ability to lock it
             */
            rc = RC ( rcDB, rcPath, rcAccessing, rcPath, rcIncorrect );
        }
    }
    return rc;
}
