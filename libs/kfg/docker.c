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

#include "docker.h"

#if CAN_HAVE_CONTAINER_ID

#include <unistd.h>
#include <sys/mman.h>

#endif

int KConfig_Get_GUID_Add_Container(  char *const value
                                   , size_t const value_size)
{
    int result = -1;
#if CAN_HAVE_CONTAINER_ID
    int const fd = open("/proc/self/cgroup", O_RDONLY);
    if (fd >= 0) {
        off_t const sfs = lseek(fd, 0, SEEK_END);
        if (sfs > 0) {
            size_t const fs = (size_t)sfs;
            void *const mm = mmap(NULL, fs, PROT_READ, MAP_FILE|MAP_SHARED, fd, 0);
            close(fd);
            if (mm != MAP_FAILED) {
                char const *const cgroup = mm;
                size_t start = 0;
                size_t end = 0;
                size_t i;

                for (i = 0; i < fs; ++i) {
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
                if (end > start && (value_size > 12) && end - start >= 12) {
                    memmove(value + (value_size - 12), &cgroup[start], 12);
                    result = 0;
                }
                munmap(mm, fs);
            }
        }
    }
#endif
    return result;
}
