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

#include <kfs/directory.h>
#include <kfs/file.h>
#include <sysalloc.h>

#define CGROUP_FILE_NAME "/proc/self/cgroup"

static char *readCGroups(size_t *const size)
{
    KDirectory *ndir = NULL;
    KFile const *fp = NULL;
    rc_t rc;

    rc = KDirectoryNativeDir(&ndir);
    assert(rc == 0 && ndir != NULL);
    rc = KDirectoryOpenFileRead(ndir, &fp, CGROUP_FILE_NAME);
    KDirectoryRelease(ndir);
    if (rc == 0) {
        uint64_t fs = 0;
        rc = KFileSize(fp, &fs);
        *size = fs;
        if (rc == 0) {
            char *const data = malloc(*size);
            if (data) {
                rc = KFileReadExactly(fp, 0, data, *size);
                if (rc == 0) {
                    pLogMsg(klogDebug, "read $(bytes)", "bytes=%zu", *size);
                    KFileRelease(fp);
                    return data;
                }
                else {
                    LogErr(klogDebug, rc, "can't read " CGROUP_FILE_NAME);
                }
                free(data);
            }
            else {
                LogMsg(klogFatal, "OUT OF MEMORY!!!");
            }
        }
        else {
            LogErr(klogFatal, rc, "can't stat " CGROUP_FILE_NAME);
        }
        KFileRelease(fp);
    }
    else {
        LogErr(klogDebug, rc, "can't open " CGROUP_FILE_NAME);
    }
    return NULL;
}

static char const *parseContainerID(char const *const cgroup, size_t const size)
{
    size_t start = 0;
    size_t end = 0;
    size_t i;


    pLogMsg(klogDebug, "cgroup\n$(id)", "id=%.*s", (int)(size), cgroup);
    for (i = 0; i < size; ++i) {
        int const ch = cgroup[i];
        if (ch == ':') {
            start = i + 1;
            continue;
        }
        if (ch == '\n') {
            end = i;
            break;
        }
    }
    if (end > start) {
        char const *id = &cgroup[start];
        size_t len = end - start;
        if (len >= 8 && strncmp(id, "/docker/", 8) == 0) {
            id += 8; len -= 8;
            pLogMsg(klogDebug, "container-id: $(id)", "id=%.*s", (int)(len), id);
            if (len >= 12) {
                return id;
            }
        }
    }
    return NULL;
}
#endif

int KConfig_Get_GUID_Add_Container(  char *const value
                                   , size_t const value_size)
{
    int result = -1;
#if CAN_HAVE_CONTAINER_ID
    if (value_size >= 12) {
        size_t fs = 0;
        char *const data = readCGroups(&fs);
        if (data) {
            char const *const id = parseContainerID(data, fs);
            if (id) {
                memmove(value + (value_size - 12), id, 12);
                result = 0;
            }
            if (result != 0) {
                LogMsg(klogDebug, "no container id found in " CGROUP_FILE_NAME);
            }
            free(data);
        }
    }
    else {
        LogMsg(klogDebug, "image guid too short");
    }
#endif
    return result;
}
