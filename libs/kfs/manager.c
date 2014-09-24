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

#include <kfs/extern.h>

#include <kfs/manager.h>
#include <kfs/path.h>
#include "path-priv.h"
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/kfs-priv.h>

#include <klib/refcount.h>
#include <klib/rc.h>

#include <krypto/key.h>
#include <krypto/encfile.h>


#include <sysalloc.h>
#include <stdlib.h>


#ifdef _DEBUGGING
#define MGR_DEBUG(msg) DBGMSG(DBG_KFS,DBG_FLAG(DBG_KFS_MGR), msg)
#else
#define MGR_DEBUG(msg)
#endif


/*--------------------------------------------------------------------------
 * KFSManager
 */
/* currently expected to be a singleton and not use a vtable but
 * be fully fleashed out here */
struct KFSManager
{
    KRefcount refcount;

    /* the current directory in the eyes of the O/S when created */
    KDirectory * cwd;

    /* the underlying perating systems view of the path of the 
     * current working directory */
    KPath * cpath;
    
};
static const char kfsmanager_classname [] = "KFSManager";

static 
KFSManager * singleton = NULL;


/* Destroy
 *  destroy file
 */
LIB_EXPORT rc_t CC KFSManagerDestroy ( KFSManager *self )
{
    if ( self == NULL )
        return RC ( rcFS, rcFile, rcDestroying, rcSelf, rcNull );

    KRefcountWhack (&self->refcount, kfsmanager_classname);

    KDirectoryRelease (self->cwd);

    KPathRelease (self->cpath);

    free (self);
    singleton = NULL;
    return 0;
}

/* AddRef
 *  creates a new reference
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KFSManagerAddRef ( const KFSManager *self )
{
    if (self != NULL)
    {
        switch (KRefcountAdd (&self->refcount, kfsmanager_classname))
        {
        case krefOkay:
            break;
        case krefZero:
            return RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcIncorrect);
        case krefLimit:
            return RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcExhausted);
        case krefNegative:
            return RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcInvalid);
        default:
            return RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcUnknown);
        }
    }
    return 0;
}

/* Release
 *  discard reference to file
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KFSManagerRelease ( const KFSManager *self )
{
    rc_t rc = 0;
    if (self != NULL)
    {
        switch (KRefcountDrop (&self->refcount, kfsmanager_classname))
        {
        case krefOkay:
        case krefZero:
            break;
        case krefWhack:
            rc = KFSManagerDestroy ((KFSManager*)self);
            break;
        case krefNegative:
            return RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcInvalid);
        default:
            rc = RC (rcFS, rcMgr, rcAttaching, rcRefcount, rcUnknown);
            break;            
        }
    }
    return rc;
}

/* OpenFileRead
 *  opens an existing file with read-only access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT rc_t CC KFSManagerOpenFileRead (const KFSManager *self,
                                           KFile const **f,
                                           const KPath * path)
{
    /* -----
     * this is a first pass that only opens files directory referenced from 
     * the ced or have a sysdir root; that is it uses KSysDir and KSysFile
     * only.
     */
    KDirectory *cwd;
    const KFile * file;
    size_t num_read;
    char pbuff [4096];
    rc_t rc;

    if ((f == NULL) || (path == NULL))
        return RC (rcFS, rcMgr, rcOpening, rcParam, rcNull);

    *f = NULL;

    if (self == NULL)
        return RC (rcFS, rcMgr, rcOpening, rcSelf, rcNull);

    rc = KPathReadPath (path, pbuff, sizeof pbuff, &num_read);
    if (rc == 0)
    {
        char rbuff [4096];

        cwd = self->cwd;
    
        rc = KDirectoryResolvePath (cwd, true, rbuff, sizeof rbuff, "%s", pbuff);
        if (rc == 0)
        {
            uint32_t type;

            type = KDirectoryPathType (cwd, "%s", rbuff);
            switch (type & ~kptAlias)
            {
            case kptNotFound:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcNotFound);
                break;

            case kptFile:
                rc = KDirectoryOpenFileRead (cwd, f, "%s", rbuff);
                break;

            case kptBadPath:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcInvalid);
                break;
            case kptDir:
            case kptCharDev:
            case kptBlockDev:
            case kptFIFO:
            case kptZombieFile:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcIncorrect);
                break;

            default:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcUnknown);
                break;
            }
        }
    }
    if (rc == 0)
    {
        size_t z;
        char obuff [4096];

        if (KPathOption (path, kpopt_encrypted, obuff, sizeof obuff, &z) == 0)
        {
            const KFile * pwfile;

            if (KPathOption (path, kpopt_pwpath, obuff, sizeof obuff, &z) == 0)
                rc = KDirectoryOpenFileRead (cwd, &pwfile, "%s", obuff);

            else if (KPathOption (path, kpopt_pwfd, obuff, sizeof obuff, &z) == 0)
                rc = KFileMakeFDFileRead (&pwfile, atoi (obuff));
            else
                rc = RC (rcFS, rcPath, rcConstructing, rcParam, rcUnsupported);

            if (rc == 0)
            {
                rc = KFileRead (pwfile, 0, obuff, sizeof obuff, &z);
                
                KFileRelease (pwfile);

                if (rc == 0)
                {
                    KKey key;
                    const KFile * encfile;

                    rc = KKeyInit (&key, kkeyAES128, obuff, z);

                    obuff[z] = '\0';

                    rc = KEncFileMakeRead (&encfile, file, &key);
                    if (rc == 0)
                    {
                        *f = encfile;
                        return 0;
                    }
                }
            }
            if (rc)
                KFileRelease (file);
        }
        else
        {
            *f = file;
            return 0;
        }
    }
    return rc;
}


/* OpenFileWrite
 *  opens an existing file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if true, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target file
 */
LIB_EXPORT rc_t CC KFSManagerOpenFileWrite (const KFSManager *self,
                                            KFile **f, bool update,
                                            const KPath * path )
{
    /* -----
     * this is a first pass that only opens files directory referenced from 
     * the ced or have a sysdir root; that is it uses KSysDir and KSysFile
     * only.
     */
    KDirectory * cwd;
    KFile * file;
    size_t num_read;
    char pbuff [4096];
    rc_t rc;

    if ((f == NULL) || (path == NULL))
        return RC (rcFS, rcMgr, rcOpening, rcParam, rcNull);

    *f = NULL;

    if (self == NULL)
        return RC (rcFS, rcMgr, rcOpening, rcSelf, rcNull);

    rc = KPathReadPath (path, pbuff, sizeof pbuff, &num_read);
    if (rc == 0)
    {
        KDirectory *cwd;
        char rbuff [4096];

        cwd = self->cwd;
    
        rc = KDirectoryResolvePath (cwd, true, rbuff, sizeof rbuff, "%s", pbuff);
        if (rc == 0)
        {
            uint32_t type;

            type = KDirectoryPathType (cwd, "%s", rbuff);
            switch (type & ~kptAlias)
            {
            case kptNotFound:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcNotFound);
                break;

            case kptFile:
                rc = KDirectoryOpenFileWrite (cwd, f, update, "%s", rbuff);
                break;

            case kptBadPath:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcInvalid);
                break;
            case kptDir:
            case kptCharDev:
            case kptBlockDev:
            case kptFIFO:
            case kptZombieFile:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcIncorrect);
                break;

            default:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcUnknown);
                break;
            }
        }
    }
    if (rc == 0)
    {
        size_t z;
        char obuff [4096];

        if (KPathOption (path, kpopt_encrypted, obuff, sizeof obuff, &z) == 0)
        {
            const KFile * pwfile;

            if (KPathOption (path, kpopt_pwpath, obuff, sizeof obuff, &z) == 0)
                rc = KDirectoryOpenFileRead (cwd, &pwfile, "%s", obuff);

            else if (KPathOption (path, kpopt_pwfd, obuff, sizeof obuff, &z) == 0)
                rc = KFileMakeFDFileRead (&pwfile, atoi (obuff));
            else
                rc = RC (rcFS, rcPath, rcConstructing, rcParam, rcUnsupported);

            if (rc == 0)
            {
                rc = KFileRead (pwfile, 0, obuff, sizeof obuff, &z);
                
                KFileRelease (pwfile);

                if (rc == 0)
                {
                    KKey key;
                    KFile * encfile;

                    rc = KKeyInit (&key, kkeyAES128, obuff, z);

                    obuff[z] = '\0';

                    rc = KEncFileMakeWrite (&encfile, file, &key);
                    if (rc == 0)
                    {
                        *f = encfile;
                        return 0;
                    }
                }
            }
            if (rc)
                KFileRelease (file);
        }
        else
        {
            *f = file;
            return 0;
        }
    }
    return rc;
}


/* CreateFile
 *  opens a file with write access
 *
 *  "f" [ OUT ] - return parameter for newly opened file
 *
 *  "update" [ IN ] - if true, open in read/write mode
 *  otherwise, open in write-only mode
 *
 *  "access" [ IN ] - standard Unix access mode, e.g. 0664
 *
 *  "mode" [ IN ] - a creation mode ( see explanation above ).
 *
 *  "path" [ IN ] KPath representing the path, URL or URN of the desired file
 */
LIB_EXPORT rc_t CC KFSManagerCreateFile ( const KFSManager *self, KFile **f,
    bool update, uint32_t access, KCreateMode mode, const KPath * path )
{
    /* -----
     * this is a first pass that only opens files directory referenced from 
     * the ced or have a sysdir root; that is it uses KSysDir and KSysFile
     * only.
     */
    KDirectory *cwd;
    KFile * file;
    size_t num_read;
    char pbuff [4096];
    rc_t rc;

    if ((f == NULL) || (path == NULL))
        return RC (rcFS, rcMgr, rcOpening, rcParam, rcNull);

    *f = NULL;

    if (self == NULL)
        return RC (rcFS, rcMgr, rcOpening, rcSelf, rcNull);

    rc = KPathReadPath (path, pbuff, sizeof pbuff, &num_read);
    if (rc == 0)
    {
        char rbuff [4096];

        cwd = self->cwd;
    
        rc = KDirectoryResolvePath (cwd, true, rbuff, sizeof rbuff, "%s", pbuff);
        if (rc == 0)
        {
            uint32_t type;

            type = KDirectoryPathType (cwd, "%s", rbuff);
            switch (type & ~kptAlias)
            {
            case kptNotFound:
            case kptFile:
                rc = KDirectoryCreateFile (cwd, &file, update, access, mode,
                                           "%s", rbuff);
                break;

            case kptBadPath:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcInvalid);
                break;
            case kptDir:
            case kptCharDev:
            case kptBlockDev:
            case kptFIFO:
            case kptZombieFile:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcIncorrect);
                break;

            default:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcUnknown);
                break;
            }
        }
    }
    if (rc == 0)
    {
        size_t z;
        char obuff [4096];

        if (KPathOption (path, kpopt_encrypted, obuff, sizeof obuff, &z) == 0)
        {
            const KFile * pwfile;

            if (KPathOption (path, kpopt_pwpath, obuff, sizeof obuff, &z) == 0)
                rc = KDirectoryOpenFileRead (cwd, &pwfile, "%s", obuff);

            else if (KPathOption (path, kpopt_pwfd, obuff, sizeof obuff, &z) == 0)
                rc = KFileMakeFDFileRead (&pwfile, atoi (obuff));
            else
                rc = RC (rcFS, rcPath, rcConstructing, rcParam, rcUnsupported);

            if (rc == 0)
            {
                rc = KFileRead (pwfile, 0, obuff, sizeof obuff, &z);
                
                KFileRelease (pwfile);

                if (rc == 0)
                {
                    KKey key;
                    KFile * encfile;

                    rc = KKeyInit (&key, kkeyAES128, obuff, z);

                    obuff[z] = '\0';

                    rc = KEncFileMakeWrite (&encfile, file, &key);
                    if (rc == 0)
                    {
                        *f = encfile;
                        return 0;
                    }
                }
            }
            if (rc)
                KFileRelease (file);
        }
        else
        {
            *f = file;
            return 0;
        }
    }
    return rc;
}


/* Remove
 *  remove an accessible object from its directory
 *
 *  "force" [ IN ] - if true and target is a directory,
 *  remove recursively
 *
 *  "path" [ IN ] - NUL terminated string in directory-native
 *  character set denoting target object
 */
LIB_EXPORT rc_t CC KFSManagerRemove ( const KFSManager *self, bool force,
                                      const KPath * path )
{
    /* -----
     * this is a first pass that only opens files directory referenced from 
     * the ced or have a sysdir root; that is it uses KSysDir and KSysFile
     * only.
     */
    size_t num_read;
    char pbuff [4096];
    rc_t rc;

    if (path == NULL)
        return RC (rcFS, rcMgr, rcOpening, rcParam, rcNull);

    if (self == NULL)
        return RC (rcFS, rcMgr, rcOpening, rcSelf, rcNull);

    rc = KPathReadPath (path, pbuff, sizeof pbuff, &num_read);
    if (rc == 0)
    {
        KDirectory *cwd;
        char rbuff [4096];

        cwd = self->cwd;
    
        rc = KDirectoryResolvePath (cwd, true, rbuff, sizeof rbuff, "%s", pbuff);
        if (rc == 0)
        {
            uint32_t type;

            type = KDirectoryPathType (cwd, "%s", rbuff);
            switch (type & ~kptAlias)
            {
            case kptNotFound:
                break;

            case kptFile:
            case kptDir:
            case kptCharDev:
            case kptBlockDev:
            case kptFIFO:
            case kptZombieFile:
                rc = KDirectoryRemove (cwd, force, "%s", rbuff);
                break;

            case kptBadPath:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcInvalid);
                break;
/*                 rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcIncorrect); */
/*                 break; */

            default:
                rc = RC (rcFS, rcMgr, rcOpening, rcFile, rcUnknown);
                break;
            }
        }
    }
    return rc;
}


/* Make
 */
LIB_EXPORT rc_t CC KFSManagerMake ( KFSManager ** pmanager )
{
    if (pmanager == NULL)
        return RC (rcFS, rcMgr, rcConstructing, rcParam, rcNull);

    if (singleton)
    {
        *pmanager = singleton;
    }
    else
    {
        KFSManager * obj;
        rc_t rc;

        obj = calloc (1, sizeof (*obj));
        if (obj == NULL)
            return RC (rcFS, rcMgr, rcConstructing, rcMemory, rcExhausted);

        KRefcountInit (&obj->refcount, 1, kfsmanager_classname, "init", 
                       kfsmanager_classname);

        rc = KDirectoryNativeDir (&obj->cwd);
        if (rc)
        {
            obj->cwd = NULL;
            KFSManagerDestroy (obj);
            return rc;
        }

        rc = KPathMakeCurrentPath (&obj->cpath);
        if (rc)
        {
            obj->cpath = NULL;
            KFSManagerDestroy (obj);
            return rc;
        }

        *pmanager = singleton = obj;

    }
    return 0;
}
