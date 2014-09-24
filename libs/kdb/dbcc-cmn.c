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
 *============================================================================
 *
 */

#include <kdb/extern.h>

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/md5.h>

#include "cc-priv.h"
#include <os-native.h>

#include <stdio.h> /* for sprintf */
#include <string.h>

static
rc_t FileCheckMD5(const KDirectory *dir, const char name[], const uint8_t digest[])
{
    rc_t rc;
    const KFile *fp;
    const KFile *mds;
    uint64_t pos;
    size_t nr;
    char buf[4096];
    
    rc = KDirectoryOpenFileRead(dir, &fp, "%s", name);
    if (rc)
        return rc;
    rc = KFileMakeMD5Read(&mds, fp, digest);
    if (rc) {
        KFileRelease(fp);
        return rc;
    }
    for (pos = 0; ; ) {
        rc = KFileRead(mds, pos, buf, sizeof(buf), &nr);
        if (rc || nr == 0)
            break;
        pos += nr;
    }
    KFileRelease(mds);

    return rc;
}

rc_t DirectoryCheckMD5(const KDirectory *dir, const char name[],
                       CCReportInfoBlock *nfo,
                       CCReportFunc report, void *ctx)
{
    rc_t rc;
    rc_t rc2 = 0;
    const KFile *kf;
    const KMD5SumFmt *sum;
    uint32_t i;
    uint32_t n;
    uint8_t digest[16];
    char pathbuf[4096];
    char mesg[1024];
    
    mesg[0] = '\0';
    
    nfo->type = ccrpt_Done;
    nfo->info.done.mesg = mesg;
    
    rc = KDirectoryOpenFileRead(dir, &kf, "%s", name);
    if (rc) {
        snprintf(mesg, sizeof(mesg), "MD5 file '%s' could not be opened", name);
        nfo->info.done.rc = rc;
        return report(nfo, ctx);
    }
    rc = KMD5SumFmtMakeRead(&sum, kf);
    if (rc) {
        KFileRelease(kf);
        snprintf(mesg, sizeof(mesg), "MD5 file '%s' could not be read", name);
        nfo->info.done.rc = rc;
        return report(nfo, ctx);
    }
    rc = KMD5SumFmtCount(sum, &n);
    if (rc)
        return rc;
    for (i = 0; i != n; ++i) {
        char *path = pathbuf;
        rc = KMD5SumFmtGet(sum, i, pathbuf, sizeof(pathbuf), digest, NULL);
        if (rc)
            break;

        /* catch case where skey.md5 contains full path */
        if ( path [ 0 ] == '/' )
        {
            size_t sz = strlen ( path );
            if ( sz >= 5 && strcmp ( & path [ sz - 5 ], "/skey" ) == 0 )
                path = "skey";
        }

        rc = FileCheckMD5(dir, path, digest);
        if (rc2 == 0)
            rc2 = rc;
        nfo->type = ccrpt_MD5;
        nfo->info.MD5.rc = rc;
        nfo->info.MD5.file = path;
        rc = report(nfo, ctx);
        if ( rc != 0 )
            break;
    }
    KMD5SumFmtRelease(sum);
    if (rc)
        return rc;
    
    nfo->type = ccrpt_Done;
    if (rc2) {
        nfo->info.done.mesg = "failed md5 validation";
        nfo->info.done.rc = rc2;
    }
    else {
        nfo->info.done.mesg = "md5 ok";
        nfo->info.done.rc = 0;
    }
    return report(nfo, ctx);
}
