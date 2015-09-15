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

rc_t ArgsConvFilepath(const Args * args, uint32_t arg_index, const char * arg, size_t arg_len, void ** result, WhackParamFnP * whack)
{
    wchar_t arg_wide[MAX_PATH];
    char * res;
    
    string_cvt_wchar_copy(arg_wide, MAX_PATH, arg, arg_len);
    
    res = rewrite_arg_as_path(arg_wide, false);
    if (!res)
        return RC (rcRuntime, rcArgv, rcConstructing, rcMemory, rcExhausted);
    
    *result = res;
    
    return 0;
}
