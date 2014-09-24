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

#if _DEBUGGING

#include <klib/extern.h>
#include <klib/debug.h>
#include "writer-priv.h"
#include <sysalloc.h>

/* PUT THIS IN A CORRECT PLACE */
#define _module(mod) \
    KDbgFlag DBG_PASTE_3(DBG_,mod,_ANY) = ( DBG_PASTE_2(mod,_CONDITIONS()) 0);
#define _condition(mod,flag) \
    (((KDbgFlag)1)<<DBG_PASTE_4(DBG_,mod,_,flag)) |

MODULE_NAMES()

#undef _module
#undef _condition

#define _module(mod)                            \
    dbg_s_flag DBG_PASTE_2(dbg_s_flag_,mod) [] = \
    {                                            \
        DBG_PASTE_2(mod,_CONDITIONS())           \
        { NULL, DBG_FLAG_NONE }                      \
    };

#define _condition(mod,flag)                                           \
    { DBG_STRING(flag), ((KDbgFlag)1)<<DBG_PASTE_4(DBG_,mod,_,flag) },

MODULE_NAMES()

#undef _module
#undef _condition

#define _module(mod)                           \
    {                                           \
        DBG_STRING(mod),                         \
        DBG_PASTE_2(dbg_s_flag_,mod),        \
        0                                       \
    },

dbg_s_mod dbg_flag_mod [] = 
{
    MODULE_NAMES()
    { NULL, NULL }
};
#undef _module

#define _module(mod) 

#include <klib/rc.h>
#include <klib/text.h>
#include <stdio.h>
#include <string.h>

KWrtHandler G_dbg_handler;


/*  ********************************************************

    setters and getters to find out what causes file-acces

    ******************************************************** */

uint64_t dbg_row_id;
char dbg_col_name[ 128 ];


LIB_EXPORT void CC KDbgSetRowId( uint64_t row_id )
{
    dbg_row_id = row_id;
}


LIB_EXPORT uint64_t CC KDbgGetRowId( void )
{
    return dbg_row_id;
}

LIB_EXPORT void CC KDbgSetColName( const char * col_name )
{
    if ( col_name != NULL )
        string_copy ( dbg_col_name, sizeof dbg_col_name, col_name, string_size( col_name ) );
    else
        dbg_col_name[ 0 ] = 0;
}

LIB_EXPORT const char * CC KDbgGetColName( void )
{
    return dbg_col_name;
}


/*  ********************************************************  */

/*
 * Init()
 * Initialize the debug messages module to a known state
 */
LIB_EXPORT rc_t CC KDbgInit( void )
{
    KDbgHandlerSetStdErr();
    dbg_row_id = 0;
    dbg_col_name[ 0 ] = 0;
    return 0;
}

LIB_EXPORT rc_t CC KDbgMsg ( const char * fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    if( (rc = vkfprintf(KDbgHandlerGet (), NULL, fmt, args)) != 0 ) {
        kfprintf(KDbgHandlerGet(), NULL, "dbgmsg failure: %R in '%s'\n", rc, fmt);
    }
    va_end ( args );

    return rc;
}

LIB_EXPORT KDbgFlag CC KDbgCondToFlag( KDbgCond cond )
{
    return (((KDbgFlag)1)<<cond);
}

/*
 * For module 'mod' turn on or off the specific flags in
 * mask flags
 *
 * bits in mask are effected.
 * return previous 
 */
LIB_EXPORT KDbgMask CC KDbgSetModConds( KDbgMod mod, KDbgMask mask, KDbgMask flags )
{
    KDbgMask previous;

    if ((mod < 0) || (mod >= DBG_MOD_COUNT))
    {
        /* unmaskable debug message */
        KDbgMsg ("%s: Undefined module: (%u)\n", __func__, mod);
        return DBG_FLAG_NONE;
    }
    previous = dbg_flag_mod[mod].flags;

    dbg_flag_mod[mod].flags = (flags & mask) | (previous & ~mask);

    return previous;
}


LIB_EXPORT bool CC KDbgTestModConds( KDbgMod mod, KDbgMask flags )
{
    if (mod >= DBG_MOD_COUNT)
    {
        /* unmaskable debug message */
        KDbgMsg ("%s: Undefined module: (%u)\n", __func__, mod);
        return false;
    }
    return DBG_FLAG_NONE != (KDbgGetModFlags (mod) & flags);
}


/*
 * Get the KDbgMod associated with a name.
 */
LIB_EXPORT rc_t CC KDbgGetModId( KDbgMod * mod, 
                   const char * mod_name, size_t mod_size )
{
    KDbgMod idx;

    for (idx = 0; idx < DBG_MOD_COUNT; ++idx)
    {
        if (strncmp (dbg_flag_mod[idx].name, mod_name, mod_size) == 0)
        {
            *mod = idx;
            return 0;
        }
    }
    *mod = DBG_MOD_NOT_FOUND;
    return RC (rcRuntime, rcLog, rcAccessing, rcParam, rcNotFound);
}


/*
 * Get the KDbgCond associated with a name.
 */
LIB_EXPORT rc_t CC KDbgGetCndFlag( KDbgMod mod, KDbgFlag * flag,
                      const char * cnd_name, size_t cnd_size )
{
    const dbg_s_flag * cnd;

    for (cnd = dbg_flag_mod[mod].conds; cnd->name != NULL; ++cnd)
    {
        if (strncmp (cnd_name, cnd->name, cnd_size) == 0)
            break;
    }
    if (cnd->name == NULL)
        return RC (rcRuntime, rcArgv, rcAccessing, rcParam, rcUndefined);

    *flag = cnd->flag;
    return 0;
}

LIB_EXPORT KDbgMask CC KDbgGetModFlags( KDbgMod mod )
{
    if ((mod < 0) || (mod >= DBG_MOD_COUNT))
    {
        /* unmaskable debug message */
        KDbgMsg ("%s: Undefined module: (%u)\n", __func__, mod);
        return DBG_FLAG_NONE;
    }
    return dbg_flag_mod[mod].flags;
}

/* 
 * param is coming in as utf-8/ASCII with NUL terminator or we fail
 * we also assume no more than 127 significant characters
 *
 * These strings can be utf-8 or ASCII even if we are using 
 * clib strXXX functions.
 */
LIB_EXPORT rc_t CC KDbgSetString( const char * string )
{
    const char * mod_s;
    const char * flag_s;
    size_t mod_z;

    KDbgMod mod;

    rc_t rc;

    mod_s = string;

    if (mod_s == NULL)
        return RC (rcRuntime, rcArgv, rcAccessing, rcParam, rcNull);
    if (*mod_s == '\0')
        return 0;

    flag_s = strchr (mod_s, '-');
    if (flag_s)
    {
        mod_z = (size_t)(flag_s - mod_s);

        if (*(++flag_s) == '\0')
            flag_s = NULL;
    }
    else
        mod_z = strlen (mod_s);

    rc = KDbgGetModId (&mod, mod_s, mod_z);
    if (rc)
        return rc;

    /* now we have a id for the module so we need to figure out the flag */

    if (flag_s == NULL)
    {
        dbg_flag_mod[mod].flags = ~(KDbgMask)0;
        rc = 0;
    }
    else
    {
        const dbg_s_flag * cnd;

        for (cnd = dbg_flag_mod[mod].conds; cnd->name != NULL; ++cnd)
        {
            if (strcmp (flag_s, cnd->name) == 0)
                break;
        }
        if (cnd->name == NULL)
            return RC (rcRuntime, rcArgv, rcAccessing, rcParam, rcUndefined);

        dbg_flag_mod[mod].flags |= cnd->flag;

    }
    return 0;
}

/* -----
 * Handlers for application and library writers.
 */

#undef KDbgHandlerSetStdOut
LIB_EXPORT rc_t CC KDbgHandlerSetStdOut( void )
{
    return KDbgHandlerSet( KWrt_DefaultWriter,KWrt_DefaultWriterDataStdOut );
}


#undef KDbgHandlerSetStdErr
LIB_EXPORT rc_t CC KDbgHandlerSetStdErr( void )
{
    return KDbgHandlerSet( KWrt_DefaultWriter,KWrt_DefaultWriterDataStdErr );
}


#undef KDbgHandlerSet
LIB_EXPORT rc_t CC KDbgHandlerSet( KWrtWriter writer, void * writer_data )
{
    G_dbg_handler.writer = writer;
    G_dbg_handler.data = writer_data;
    return 0;
}

#undef KDbgWriterDataGet
LIB_EXPORT void * CC KDbgWriterDataGet( void )
{
    return ( KDbgHandlerGet()->data );
}


#undef KDbgWriterGet
LIB_EXPORT KWrtWriter CC KDbgWriterGet( void )
{
    return ( KDbgHandlerGet()->writer );
}


#undef KDbgHandlerGet
LIB_EXPORT KWrtHandler * CC KDbgHandlerGet( void )
{
    return ( &G_dbg_handler );
}

#endif /* _DEBUGGING */
