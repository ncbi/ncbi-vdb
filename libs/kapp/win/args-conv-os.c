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

#include <klib/rc.h>
#include <kapp/args-conv.h>

#include <os-native.h>

#include "main-priv-win.h"

static
char* convert_arg_utf8(const wchar_t* arg)
{
    size_t src_size, dst_size;
    char* utf8;
    /* measure the string */
    wchar_cvt_string_measure(arg, &src_size, &dst_size);

    /* allocate a UTF-8 buffer */
    utf8 = malloc(dst_size + 1);
    if (utf8 != NULL)
    {
        /* copy the wide argument to utf8 */
        wchar_cvt_string_copy(utf8, dst_size + 1,
            arg, src_size);

        /* terminate the string */
        utf8[dst_size] = 0;
    }

    return utf8;
}

char* CC rewrite_arg_as_path(const wchar_t* arg)
{
    char* utf8;
    bool has_drive = false;
    size_t i, dst_size;
    DWORD len;

    /* detect drive or full path */
    wchar_t rewrit[MAX_PATH];
    if (arg[0] < 128)
    {
        bool rewrite = false;

        /* look for non-drive path */
        if (arg[0] == '\\' || arg[0] == '/')
        {
            /* full path - not network */
            if (arg[1] != '\\' && arg[1] != '/')
            {
                /* check for cygdrive */
                if (memcmp(arg, L"/cygdrive/", sizeof L"/cygdrive/" - sizeof L"") == 0)
                    arg += sizeof "/cygdrive" - 1;
                else
                    rewrite = true;

            }

        }
        /* look for drive path */
        else if (isalpha(arg[0]) && arg[1] == ':')
        {
            has_drive = true;

            /* look for drive relative */
            if (arg[2] != '\\' && arg[2] != '/')
                rewrite = true;
        }
        if (rewrite)
        {
            /* incomplete path */
            len = GetFullPathNameW(arg, sizeof rewrit / sizeof rewrit[0], rewrit, NULL);
            if (len == 0 || len >= MAX_PATH)
            {
                /* complain */
                return NULL;
            }
            arg = rewrit;

            has_drive = (isalpha(arg[0]) && arg[1] == ':');
        }
    }

    /* allocate a UTF-8 buffer */
    utf8 = convert_arg_utf8(arg);
    if (utf8 != NULL)
    {
        dst_size = string_size(utf8);
        if (has_drive)
        {
            utf8[1] = utf8[0];
            utf8[0] = '/';
        }

        /* map all backslashes to fwdslashes */
        for (i = 0; i < dst_size; ++i)
        {
            if (utf8[i] == '\\')
                utf8[i] = '/';
        }
    }

    return utf8;
}

int ConvertWArgsToUtf8(int argc, wchar_t* wargv[], char** argv[])
{
    int status = 0;
    /* create a copy of args */
    *argv = calloc(argc + 1, sizeof * *argv);
    if (argv == NULL)
        status = 5;
    else
    {
        int i;
        /* convert wchar_t arguments to UTF-8
           rewriting anything that looks like a path */
        for (i = 0; i < argc; ++i)
        {
            (*argv)[i] = rewrite_arg_as_path(wargv[i]);

            if ((*argv)[i] == NULL)
                break;
        }

        /* perform normal main operations on UTF-8 with POSIX-style paths */
        if (i != argc)
        {
            status = 6;
        }
    }
    return status;
}

rc_t ArgsConvFilepath(const Args* args, uint32_t arg_index, const char* arg, size_t arg_len, void** result, WhackParamFnP* whack)
{
    wchar_t arg_wide[MAX_PATH];
    char* res;

    string_cvt_wchar_copy(arg_wide, MAX_PATH, arg, arg_len);

    res = rewrite_arg_as_path(arg_wide);
    if (!res)
        return RC(rcRuntime, rcArgv, rcConstructing, rcMemory, rcExhausted);

    *result = res;

    return 0;
}

