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

#include <klib/defs.h>
#include <klib/log.h>

#include "docker.h"

#if CAN_HAVE_CONTAINER_ID

#include <stdio.h>
#include <unistd.h>

#include <kfs/file.h>
#include <kfs/directory.h>

#define CGROUP_FILE_PATH "/proc/self/cgroup"
/* /proc/self/cgroup lines like this:
 * 14:name=systemd:/docker/2b644fadb9fbf627caeede280d3d00e9c4cc59022ae4ee186a92c41e96eb8106
 * ^ unimportant ^         ^--------------- this part is the container id ----------------^
 */
#endif

int KConfig_Get_GUID_Add_Container(  char *const value
                                   , size_t const value_size)
{
    if (value_size >= 12) {
#if CAN_HAVE_CONTAINER_ID
        char buffer[4096];
        size_t inbuf = 0;
        size_t nread = 0;
        uint64_t pos = 0;
        KFile const *fp = NULL;
        KDirectory *ndir = NULL;
        rc_t rc = 0;
        bool good = false;

        rc = KDirectoryNativeDir(&ndir);
        assert(rc == 0);

        rc = KDirectoryOpenFileRead(ndir, &fp, CGROUP_FILE_PATH);
        KDirectoryRelease(ndir);
        if (rc) {
            LogErr(klogDebug, rc, "can not open " CGROUP_FILE_PATH);
            return -1;
        }
        while (good == false && (rc = KFileRead(fp, pos, buffer + inbuf, sizeof(buffer) - inbuf, &nread)) == 0 && nread > 0)
        {
            size_t i = 0, start = 0;

            pos += nread;
            inbuf += nread;
            for (i = 0; i < inbuf; ) {
                int const ch = buffer[i++];
                if (ch == ':') {
                    start = i;
                    continue;
                }
                if (ch == '\n') {
                    char const *id = &buffer[start];
                    size_t len = (i - 1) - start;
                    if (len >= 8 && strncmp(id, "/docker/", 8) == 0) {
                        id += 8; len -= 8;
                        pLogMsg(klogDebug, "container-id: $(id)", "id=%.*s", (int)(len), id);
                        if (len >= 12) {
                            memmove(value + (value_size - 12), id, 12);
                            good = true;
                            break;
                        }
                    }
                    inbuf -= i;
                    memmove(buffer, buffer + i, inbuf);
                    i = 0;
                }
            }
        }
        KFileRelease(fp);
        return good ? 0 : -1;
    }
#endif
    return -1;
}
