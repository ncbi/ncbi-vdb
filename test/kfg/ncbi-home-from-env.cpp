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

#include <climits> /* PATH_MAX */
#include <cstdlib> // system
#include <cstring> // strrchr
 #include <stdio.h>

int main(int argc, char **argv) {
    char *last = strrchr(argv[0], '/');
    if (!last)
        return 1;
    char *n = strstr(argv[0], "ncbi-vdb");
    int s = 8;
    if (!n)
        return 1;
    int l = strlen(argv[0]);
    l -= n - argv[0];
    int tail = strlen(argv[0]);
    tail -= last - argv[0];
    size_t sPfx = n - argv[0];
    char command[PATH_MAX] = "";
    sprintf(command,
        "./ncbi-home-from-env.sh %.*ssra-tools%.*s/../bin/vdb-config",
        sPfx, argv[0], l - tail - s, n + s);
    return system(command) != 0;
}
