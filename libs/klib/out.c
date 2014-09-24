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
#include <sysalloc.h>

#include <stdarg.h>

LIB_EXPORT KWrtHandler G_out_handler;

LIB_EXPORT rc_t CC KOutInit ( void )
{
    rc_t rc;

    rc = KOutHandlerSetStdOut();

    return rc;
}

LIB_EXPORT rc_t CC KOutMsg ( const char * fmt, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, fmt );

    if( (rc = vkfprintf(KOutHandlerGet(), NULL, fmt, args)) != 0 ) {
        kfprintf(KOutHandlerGet(), NULL, "outmsg failure: %R in '%s'\n", rc, fmt);
    }

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
