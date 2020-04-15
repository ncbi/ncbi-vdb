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

size_t KWrtFmt_rc_t ( char * pout, size_t max, const char * fmt, rc_t rc_in )
{
    rc_t rc;
    const char * str;
    const char * mod_str;
    const char * targ_str;
    const char * ctx_str;
    const char * obj_str;
    const char * state_str;
    size_t needed;
    size_t mod_size;
    size_t targ_size;
    size_t ctx_size;
    size_t obj_size;
    size_t state_size;
    enum RCModule mod;
    enum RCTarget targ;
    enum RCContext ctx;
    int obj;
    enum RCState state;

    assert (pout);
    assert (fmt);

    if (*fmt == '#')
        rc = RCExplain (rc_in, pout, max, &needed);

    else if (rc_in != 0)
    {
        mod = GetRCModule (rc_in);
        targ = GetRCTarget (rc_in);
        ctx = GetRCContext (rc_in);
        obj = GetRCObject (rc_in);
        state = GetRCState (rc_in);

        if ((mod < 0) || (mod > rcLastModule_v1_2))
            mod = rcLastModule_v1_2;

        if ((targ < 0) || (targ > rcLastTarget_v1_2))
            targ = rcLastTarget_v1_2;

        if ((ctx < 0) || (ctx > rcLastContext_v1_1))
            ctx = rcLastContext_v1_1;

        if ((obj < 0) || (obj > rcLastObject_v1_2))
            obj = rcLastObject_v1_2;

        if ((state < 0) || (state > rcLastState_v1_1))
            state = rcLastState_v1_1;

        mod_str = RCModuleStrings[mod];
        str = strchrnul(mod_str, ' ');
        mod_size = str - mod_str;

        targ_str = RCTargetStrings[targ];
        str = strchrnul(targ_str, ' ');
        targ_size = str - targ_str;

        ctx_str = RCContextStrings[ctx];
        str = strchrnul(ctx_str, ' ');
        ctx_size = str - ctx_str;

/* object is tricky because it overlaps target */
        if (obj == 0)
        {
            obj_str = RCObjectStrings[obj];
            str = strchrnul(obj_str, ' ');
            obj_size = str - obj_str;
        }
        else if (obj < rcLastTarget_v1_1)
        {
            obj_str = RCTargetStrings[obj];
            str = strchrnul(obj_str, ' ');
            obj_size = str - obj_str;
        }
        else
        {
            obj -= (rcLastTarget_v1_1-1);
            obj_str = RCObjectStrings[obj];
            str = strchrnul(obj_str, ' ');
            obj_size = str - obj_str;
        }

        state_str = RCStateStrings[state];
        str = strchrnul(state_str, ' ');
        state_size = str - state_str;

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
        state = GetRCState (rc_in);
        state_str = RCStateStrings[state];
        str = strchrnul(state_str, ' ');
        state_size = str - state_str;
        rc = string_printf (pout, max, & needed, "RC(%*s)",
                           (uint32_t)state_size, state_str);
    }

    return rc != 0 ? 0 : needed;
}
