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

#endif

int KConfig_Get_GUID_Add_Container(  char *const value
                                   , size_t const value_size)
{
    if (value_size >= 12) {
#if CAN_HAVE_CONTAINER_ID
        char line[1024];
        snprintf(line, sizeof(line), "/proc/%i/cgroup", getpid());
        FILE *fp = fopen(line, "r");
        if (fp) {
            pLogMsg(klogDebug, "opened $(line)", "line=%s", line);
            while (fgets(line, sizeof(line), fp) != NULL) {
                int start = 0;
                int end = 0;
                int i;
                pLogMsg(klogDebug, "$(line)", "line=%s", line);
                for (i = 0; i < sizeof(line); ++i) {
                    int const ch = line[i];
                    if (ch == '\0')
                        break;
                    if (ch == ':') {
                        start = i + 1;
                        continue;
                    }
                    if (ch == '\n') {
                        end = i;
                        break;
                    }
                }
                if (end < start) {
                    char const *id = &line[start];
                    size_t len = end - start;
                    if (len >= 8 && strncmp(id, "/docker/", 8) == 0) {
                        id += 8; len -= 8;
                        pLogMsg(klogDebug, "container-id: $(id)", "id=%.*s", (int)(len), id);
                        if (len >= 12) {
                            memmove(value + (value_size - 12), id, 12);
                            fclose(fp);
                            return 0;
                        }
                    }
                }
            }
            fclose(fp);
        }
    }
#endif
    return -1;
}
