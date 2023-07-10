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

// need for strchrnul
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

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

static void Get_RC_String_Module(struct RC_String *out, rc_t const rc)
{
    out->field = rcf_module;
    out->text = Get_RC_String(&out->size, out->value = (int)GetRCModule(rc), RCModuleStrings, rcLastModule_v1_2);
}

static void Get_RC_String_Target(struct RC_String *out, rc_t const rc)
{
    out->field = rcf_target;
    out->value = GetRCTarget(rc);
    out->text = INVALID;

    if (out->value >= rcNoTarg && out->value <= rcUri)
        out->text = RCTargetStrings[out->value];
    else if (out->value == rcProvider)
        out->text = RCTargetStrings[rcProvider];

    out->size = measure(out->text);
}

static void Get_RC_String_Context(struct RC_String *out, rc_t const rc)
{
    out->field = rcf_context;
    out->text = Get_RC_String(&out->size, out->value = (int)GetRCContext(rc), RCContextStrings, rcLastContext_v1_1);
}

static void Get_RC_String_Object(struct RC_String *out, rc_t const rc)
{
    out->field = rcf_object;
    out->value = GetRCObject(rc);
    out->text = INVALID;

    if (out->value == rcNoObj)
        out->text = RCObjectStrings[rcNoObj];
    else if (out->value > 0) {
        if (out->value <= rcUri)
            out->text = RCTargetStrings[out->value];
        else if (out->value < rcLastObject_v1_2)
            out->text = RCObjectStrings[out->value - rcUri];
    }
    out->size = measure(out->text);
}

static void Get_RC_String_State(struct RC_String *out, rc_t const rc)
{
    out->field = rcf_state;
    out->text = Get_RC_String(&out->size, out->value = (int)GetRCState(rc), RCStateStrings, rcLastState_v1_1);
}

LIB_EXPORT void CC Get_RC_Strings(rc_t rc, struct RC_String out[5])
{
    Get_RC_String_Module(&out[rcf_module], rc);
    Get_RC_String_Target(&out[rcf_target], rc);
    Get_RC_String_Context(&out[rcf_context], rc);
    Get_RC_String_Object(&out[rcf_object], rc);
    Get_RC_String_State(&out[rcf_state], rc);
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
        struct RC_String out[5];

        Get_RC_Strings(rc_in, out);
        rc = string_printf (pout, max, & needed,
#if _DEBUGGING
                            "RC(%s:%u:%s %*s,%*s,%*s,%*s,%*s)", GetRCFilename(), GetRCLineno(), GetRCFunction(),
#else
                            "RC(%*s,%*s,%*s,%*s,%*s)",
#endif
                            (uint32_t)out[rcf_module].size, out[rcf_module].text,
                            (uint32_t)out[rcf_target].size, out[rcf_target].text,
                            (uint32_t)out[rcf_context].size, out[rcf_context].text,
                            (uint32_t)out[rcf_object].size, out[rcf_object].text,
                            (uint32_t)out[rcf_state].size, out[rcf_state].text);
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

#include "rc-asserts.h"

/* creates a typedef for a char array of length 1 or -1,
ß * the compiler will complain if the length is less than zero.
 */
#define STATIC_ASSERT(MEMO, EXPR) typedef char BAD_ ## MEMO [((EXPR) ? (1) : (-1))]

STATIC_ASSERT(RC_MODULE_COUNT, ASSERTION_RC_MOD_COUNT);
STATIC_ASSERT(RC_TARGET_COUNT, ASSERTION_RC_TARG_COUNT);
STATIC_ASSERT(RC_CONTEXT_COUNT, ASSERTION_RC_CTX_COUNT);
STATIC_ASSERT(RC_OBJECT_COUNT, ASSERTION_RC_OBJ_COUNT);
STATIC_ASSERT(RC_STATE_COUNT, ASSERTION_RC_STATE_COUNT);

STATIC_ASSERT(RC_MODULE_EXTENSION, ASSERTION_RC_MOD_EXTENSION);
STATIC_ASSERT(RC_TARGET_EXTENSION, ASSERTION_RC_TARG_EXTENSION);
STATIC_ASSERT(RC_CONTEXT_EXTENSION, ASSERTION_RC_CTX_EXTENSION);
STATIC_ASSERT(RC_OBJECT_EXTENSION, ASSERTION_RC_OBJ_EXTENSION);
STATIC_ASSERT(RC_STATE_EXTENSION, ASSERTION_RC_STATE_EXTENSION);
