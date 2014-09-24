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

#include <vfs/keyring-priv.h>

#include <kfg/config.h>

#include <klib/text.h>
#include <klib/rc.h>
#include <klib/log.h>
#include <klib/printf.h>

#include <kfs/file.h>
#include <kfs/directory.h>
#include <kfs/lockfile.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <stdio.h>

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

const char* KeyRingDefaultDataDir = "~/.ncbi";

/*TODO: move to ../keyring.c */
LIB_EXPORT bool CC KKeyRingIsServerRunning(const char* dataDir)
{   
    KDirectory* wd;
    rc_t rc = KDirectoryNativeDir (&wd);
    if (rc == 0)
    {
        char lockFileName[MAX_PATH];
        if (dataDir == NULL)
            dataDir = KeyRingDefaultDataDir;
        rc = string_printf(lockFileName, sizeof(lockFileName)-1, NULL, "%s/keyring_lock", dataDir);
        
        if (rc == 0)
        {
            KFile* lockedFile;
            rc = KDirectoryCreateExclusiveAccessFile(wd, &lockedFile, true, 0600, kcmOpen, "%s", lockFileName);
            if (rc == 0)
                KFileRelease(lockedFile);
        }
        KDirectoryRelease(wd);
    }
    return rc != 0;
}

rc_t StartKeyRing(const char* dataDir)
{
    rc_t rc = 0;
    
    pid_t child = fork();
    switch (child)
    {
        case 0: /* child */
        {   /* become the server */
        
/*TODO: calculate based on $(APPPATH) in kfg */
const char* KeyRingServerExeName = "/home/boshkina/internal/asm-trace/centos/gcc/stat/x86_64/dbg/bin/keyring-srv";

            if (dataDir == NULL)
                dataDir = "~/.ncbi";
            LogMsg(klogInfo, "Keyring: execl...");

            if (execl(KeyRingServerExeName, KeyRingServerExeName, dataDir, NULL) == -1)
            {   /* TODO: look around:
                    - same dir as the current executable (kfg/APPPATH)
                    - current dir
                    - etc.
                */
            }
            pLogMsg(klogErr, 
                    "Keyring: execl($(exe)) failed ($(errno)=$(perrno))", 
                    "exe=%s,errno=%d,perrno=%!", 
                    KeyRingServerExeName, errno, errno);
            exit(1);
            break;
        }
        case -1: /* error */
        {
            switch (errno)
            {
            case EAGAIN:
            case ENOMEM:
                rc = RC (rcVFS, rcProcess, rcProcess, rcMemory, rcInsufficient);
                break;
            case ENOSYS:
                rc = RC (rcVFS, rcProcess, rcProcess, rcInterface, rcUnsupported);
                break;
            default:
                rc = RC (rcVFS, rcProcess, rcProcess, rcError, rcUnknown);
                break;
            }
            break;
        }
        default: /* parent */
            break;
    }
        
    return rc;
}

#if 0
static
rc_t GetAppPath(const char* buf, size_t bufsize)
{
    KConfig* kfg;
    rc_t rc = KConfigMake(&kfg, NULL);
    if (rc == 0)
    {
        const KConfigNode *node;
        char path[] = "APPPATH";
        char buf[4096];
        size_t num_read;
        rc_t rc2;
    
        rc_t rc=KConfigOpenNodeRead(kfg, &node, path, string_measure(path, NULL), "%s", buf);
        if (rc == 0) 
        {
            rc = KConfigNodeRead(node, 0, buf, bufsize, &num_read, NULL);
            rc2 = KConfigNodeRelease(node);
            if (rc == 0)
                rc = r2;
        }
        rc2 = KConfigRelease(kfg);
        if (rc == 0)
            rc = r2;
    }
    return rc;
}
#endif
