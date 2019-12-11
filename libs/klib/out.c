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
*/

#include <klib/extern.h>
#include <klib/out.h>
#include "writer-priv.h"
#include <klib/rc.h>
#include <klib/text.h>
#include <sysalloc.h>

#include <stdarg.h>
#include <string.h>

LIB_EXPORT KWrtHandler G_out_handler;

LIB_EXPORT rc_t CC KOutInit ( void )
{
    return KOutHandlerSetStdOut ();
}

static
rc_t KOutMsgNulTermStringFmt ( const char * arg )
{
    size_t num_writ;
    KWrtHandler * kout_msg_handler = KOutHandlerGet ();
    return ( * kout_msg_handler -> writer ) ( kout_msg_handler -> data, arg, string_size ( arg ), & num_writ );        
}

static
rc_t KOutMsgPrecNulTermStringFmt ( va_list args )
{
    unsigned int arg1 = va_arg ( args, unsigned int );
    const char* arg2 = va_arg ( args, const char* );

    size_t num_writ;
    KWrtHandler * kout_msg_handler = KOutHandlerGet ();
    return ( * kout_msg_handler -> writer ) ( kout_msg_handler -> data, arg2, arg1, & num_writ );        
}

static
rc_t KOutMsgStringFmt ( const String * arg )
{
    size_t num_writ;
    KWrtHandler * kout_msg_handler = KOutHandlerGet ();
    return ( * kout_msg_handler -> writer ) ( kout_msg_handler -> data, arg->addr, arg->size, & num_writ );
}

static
rc_t KOutMsgCharFmt ( uint32_t u32 )
{
    rc_t rc;
    size_t num_writ;
    KWrtHandler * kout_msg_handler = KOutHandlerGet ();

    if ( u32 < 128 )
    {
        char ch = ( char ) u32;
        rc = ( * kout_msg_handler -> writer ) ( kout_msg_handler -> data, &ch, 1, & num_writ );
    }
    else
    {
        char buf[4];
        int dbytes = utf32_utf8 ( buf, & buf [ sizeof buf ], u32 );
        if ( dbytes <= 0 )
        {   /* invalid character */
            rc = ( * kout_msg_handler -> writer ) ( kout_msg_handler -> data, "?", 1, & num_writ );
        }
        else
        {
            rc = ( * kout_msg_handler -> writer ) ( kout_msg_handler -> data, buf, dbytes, & num_writ );
        }
    }

    return rc;
}

/* Prevent calling memcmp(s1, s2, n) when sizeof (s1) < n */
static
int match_format(const char * format, const char * literal, size_t s)
{
    static const size_t MAX = 5;

    assert(s <= MAX);

    if (format == NULL)
        return 1;
    else {
        size_t x = 0;

        for (x = 0; x < MAX - 1; ++x)
            if (format[x] == '\0')
                break;
        ++x;

        if (x < s)
            return x;
        else
            return memcmp(format, literal, s);
    }
}

LIB_EXPORT rc_t CC KOutVMsg ( const char * fmt, va_list args )
{
    rc_t rc = 0;

#define MATCH_FORMAT(format, literal) \
    ( ( const void* ) ( format ) == ( const void* ) ( literal ) )

    /* rapid pointer comparison */
    if ( MATCH_FORMAT ( fmt, "%s" ) )
        rc = KOutMsgNulTermStringFmt ( va_arg ( args, const char * ) );
    else if ( MATCH_FORMAT ( fmt, "%.*s" ) )
        rc = KOutMsgPrecNulTermStringFmt ( args );
    else if ( MATCH_FORMAT ( fmt, "%S" ) )
        rc = KOutMsgStringFmt ( va_arg ( args, const String * ) );
    else if ( MATCH_FORMAT ( fmt, "%c" ) )
        rc = KOutMsgCharFmt ( va_arg ( args, unsigned int ) );

#undef MATCH_FORMAT
#define MATCH_FORMAT(format, literal) \
    ( match_format ( ( format ), ( literal ), sizeof ( literal ) ) == 0 )

    /* slower value comparison */
    else if (MATCH_FORMAT(fmt, "%s"))
        rc = KOutMsgNulTermStringFmt ( va_arg ( args, const char * ) );
    else if (MATCH_FORMAT(fmt, "%.*s"))
        rc = KOutMsgPrecNulTermStringFmt ( args );
    else if (MATCH_FORMAT(fmt, "%S"))
        rc = KOutMsgStringFmt ( va_arg ( args, const String * ) );
    else if (MATCH_FORMAT(fmt, "%c"))
        rc = KOutMsgCharFmt ( va_arg ( args, unsigned int ) );
    else if( (rc = vkfprintf(KOutHandlerGet(), NULL, fmt, args)) != 0 ) 
    {
        kfprintf(KOutHandlerGet(), NULL, "outmsg failure: %R in '%s'\n", rc, fmt);
    }
#undef MATCH_FORMAT

    return rc;
}

LIB_EXPORT rc_t CC KOutMsg ( const char * fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    rc = KOutVMsg ( fmt, args );

    va_end ( args );

    return rc;
}

/* kprintf
 *  performs a printf to our output writer
 *  much like KOutMsg, but returns a size_t
 */
LIB_EXPORT rc_t CC kprintf ( size_t *num_writ, const char * fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    rc = vkfprintf ( KOutHandlerGet (), num_writ, fmt, args );

    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC vkprintf ( size_t *num_writ, const char * fmt, va_list args )
{
    return vkfprintf ( KOutHandlerGet (), num_writ, fmt, args );
}


#undef KOutHandlerSetStdOut
LIB_EXPORT rc_t CC KOutHandlerSetStdOut ( void )
{
    return KOutHandlerSet( KWrt_DefaultWriter, KWrt_DefaultWriterDataStdOut );
}


#undef KOutHandlerSetStdErr
LIB_EXPORT rc_t CC KOutHandlerSetStdErr ( void )
{
    return KOutHandlerSet( KWrt_DefaultWriter, KWrt_DefaultWriterDataStdErr );
}


#undef KOutHandlerSet
LIB_EXPORT rc_t CC KOutHandlerSet ( KWrtWriter writer, void * writer_data )
{
    G_out_handler.writer = writer;
    G_out_handler.data = writer_data;
    return 0;
}


#undef KOutDataGet
LIB_EXPORT void * CC KOutDataGet ( void )
{
    return ( KOutHandlerGet()->data );
}


#undef KOutWriterGet
LIB_EXPORT KWrtWriter CC KOutWriterGet ( void )
{
    return ( KOutHandlerGet()->writer );
}


#undef KOutHandlerGet
LIB_EXPORT KWrtHandler * CC KOutHandlerGet ( void )
{
    return ( &G_out_handler );
}
