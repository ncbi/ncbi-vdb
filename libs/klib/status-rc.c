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

#include <klib/extern.h>
#include "writer-priv.h"
#include <klib/writer.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <sysalloc.h>
#include <os-native.h> /* for strchrnul on non-linux */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

static size_t measure(char const *const str)
{
    assert(str != NULL);
    return (size_t)(strchrnul(str, ' ') - str);
}

static char const *const INVALID = "INVALID";
static char const *Get_RC_String(size_t *const outsize, int const value, char const *const *const strings, int const limit)
{
    char const *const result = (0 <= value && value < limit) ? strings[value] : INVALID;
    *outsize = measure(result);
    return result;
}

static char const *Get_RC_String_Module(size_t *const outsize, rc_t const rc)
{
    return Get_RC_String(outsize, (int)GetRCModule(rc), RCModuleStrings, rcLastModule_v1_2);
}

static char const *Get_RC_String_Target(size_t *const outsize, rc_t const rc)
{
    int const value = GetRCTarget(rc);
    char const *result = INVALID;

    if (value >= rcNoTarg && value <= rcUri)
        result = RCTargetStrings[value];
    else if (value == rcProvider)
        result = RCTargetStrings[rcProvider];
    *outsize = measure(result);
    return result;
}

static char const *Get_RC_String_Context(size_t *const outsize, rc_t const rc)
{
    return Get_RC_String(outsize, (int)GetRCContext(rc), RCContextStrings, rcLastContext_v1_1);
}

static char const *Get_RC_String_Object(size_t *const outsize, rc_t const rc)
{
    int const value = GetRCObject(rc);
    char const *result = INVALID;

    if (value == rcNoObj)
        result = RCObjectStrings[rcNoObj];
    else if (value > 0) {
        if (value <= rcUri)
            result = RCTargetStrings[value];
        else if (value < rcLastObject_v1_2)
            result = RCObjectStrings[value - rcUri];
    }
    *outsize = measure(result);
    return result;
}

static char const *Get_RC_String_State(size_t *const outsize, rc_t const rc)
{
    return Get_RC_String(outsize, (int)GetRCState(rc), RCStateStrings, rcLastState_v1_1);
}

size_t KWrtFmt_rc_t ( char * pout, size_t max, const char * fmt, rc_t rc_in )
{
    rc_t rc = 0;
    size_t needed = 0;

    assert (pout);
    assert (fmt);

    if (*fmt == '#')
        rc = RCExplain (rc_in, pout, max, &needed);

    else if (rc_in != 0)
    {
        size_t mod_size = 0;
        size_t targ_size = 0;
        size_t ctx_size = 0;
        size_t obj_size = 0;
        size_t state_size = 0;
        char const *const mod_str = Get_RC_String_Module (&mod_size  , rc_in);
        char const *const targ_str = Get_RC_String_Target(&targ_size , rc_in);
        char const *const ctx_str = Get_RC_String_Context(&ctx_size  , rc_in);
        char const *const obj_str = Get_RC_String_Object (&obj_size  , rc_in);
        char const *const state_str = Get_RC_String_State(&state_size, rc_in);

#if _DEBUGGING
        rc = string_printf (pout, max, & needed, "RC(%s:%u:%s %*s,%*s,%*s,%*s,%*s)", 
                            GetRCFilename(), GetRCLineno(), GetRCFunction(),
                           (uint32_t)mod_size, mod_str,
                           (uint32_t)targ_size, targ_str,
                           (uint32_t)ctx_size, ctx_str,
                           (uint32_t)obj_size, obj_str,
                           (uint32_t)state_size, state_str);
#else
        rc = string_printf (pout, max, & needed, "RC(%*s,%*s,%*s,%*s,%*s)", 
                           (uint32_t)mod_size, mod_str,
                           (uint32_t)targ_size, targ_str,
                           (uint32_t)ctx_size, ctx_str,
                           (uint32_t)obj_size, obj_str,
                           (uint32_t)state_size, state_str);
#endif
    }
    else
    {
        int state = (int)GetRCState (rc_in);
        char const *state_str = RCStateStrings[state];
        size_t const state_size = measure(state_str);
        rc = string_printf (pout, max, & needed, "RC(%*s)",
                           (uint32_t)state_size, state_str);
    }

    return rc != 0 ? 0 : needed;
}
