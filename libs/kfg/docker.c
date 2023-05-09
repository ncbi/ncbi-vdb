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
#include <inttypes.h>

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
#if CAN_HAVE_CONTAINER_ID
    if (value_size >= 12) {
        char buffer[32];
        //size_t inbuf = 0;
        size_t nread = 0;
        //uint64_t pos = 0;
        KFile const *fp = NULL;
        KDirectory *ndir = NULL;
        rc_t rc = 0;
        //bool good = false;

        rc = KDirectoryNativeDir(&ndir);
        assert(rc == 0);

        /* In a docker container, hostname is the short version of the container
         * id, it is 12 hex digits
         */
        rc = KDirectoryOpenFileRead(ndir, &fp, "/etc/hostname");
        KDirectoryRelease(ndir);
        if (rc) {
            LogErr(klogDebug, rc, "can not open /etc/hostname");
            return -1;
        }
        rc = KFileRead(fp, 0, buffer, sizeof(buffer), &nread);
        KFileRelease(fp);
        if (rc == 0 && nread >= 12) {
            uint64_t hex = 0;
            int n = 0;

            if (sscanf(buffer, "%" SCNx64 "%n", &hex, &n) == 1 && n >= 12) {
                memmove(value + (value_size - 12), buffer, 12);
                return 0;
            }
        }
    }
#endif
    return -1;
}
