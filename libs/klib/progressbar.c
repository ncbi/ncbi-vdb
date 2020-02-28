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
#include <klib/rc.h>
#include <klib/printf.h>
#include <klib/progressbar.h>
#include "writer-priv.h"    /* for sys_simple_write() and sys_is_a_tty() */

#include <sysalloc.h>
#include <stdlib.h>

#define MAX_DIGITS 2
#define BUFFER_SIZE 64
#define STDOUT_FD 1
#define STDERR_FD 2

typedef struct progressbar
{
    char buffer[ BUFFER_SIZE ];
    percent_t percent;
    bool initialized;
    void * out_writer;
    uint8_t digits;
} progressbar;

static rc_t make_progressbar_cmn( progressbar ** pb, const uint8_t digits, bool use_stderr )
{
    rc_t rc = 0;
    if ( pb == NULL )
        rc = RC( rcVDB, rcNoTarg, rcConstructing, rcSelf, rcNull );
    else if ( digits > MAX_DIGITS )
        rc = RC( rcVDB, rcNoTarg, rcConstructing, rcParam, rcExcessive);
    else
    {
        void * h_stdout;
        void * h_stderr;
        
        *pb = NULL;
        rc = KWrtSysInit( &h_stdout, &h_stderr );
        if ( rc == 0 )
        {
            progressbar	* p = calloc( 1, sizeof( *p ) );
            if ( p == NULL )
                rc = RC( rcVDB, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
            else
            {
                p -> digits = digits;
                p -> out_writer = use_stderr ? h_stderr : h_stdout;
                *pb = p;
            }
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC make_progressbar( progressbar ** pb, const uint8_t digits )
{
    return make_progressbar_cmn( pb, digits, false );
}

LIB_EXPORT rc_t CC make_progressbar_stderr( struct progressbar ** pb, const uint8_t digits )
{
    return make_progressbar_cmn( pb, digits, true );
}

rc_t CC KWrt_DefaultWriter( void * self, const char * buffer, size_t bufsize, size_t * num_writ );

static rc_t write_buffer( progressbar * pb, size_t to_write )
{
    size_t written;
    rc_t rc = KWrt_DefaultWriter( pb -> out_writer, pb -> buffer, to_write, &written );
    /* size_t printed = sys_simple_write( pb -> out_fd, pb -> buffer, to_write ); */
    if ( rc == 0 && to_write != written )
        rc = RC( rcVDB, rcNoTarg, rcWriting, rcRange, rcInvalid );
    return rc;
}

static rc_t print_newline( progressbar * pb )
{
    size_t num_writ;
    rc_t rc = string_printf( pb->buffer, BUFFER_SIZE, &num_writ, "\n" );
    if ( rc == 0 )
        rc = write_buffer( pb, num_writ );
    return rc;
}

LIB_EXPORT rc_t CC destroy_progressbar( progressbar * pb )
{
    if ( pb == NULL )
        return 0;
    print_newline( pb );
    free( pb );
    return 0;
}

static rc_t print_progress_1( progressbar * pb, const char * fmt, percent_t value )
{
    size_t num_writ;
    rc_t rc = string_printf( pb->buffer, BUFFER_SIZE, &num_writ, fmt, value );
    if ( rc == 0 )
        rc = write_buffer( pb, num_writ );
    return rc;
}

static rc_t print_progress_2( progressbar * pb, const char * fmt, percent_t value1, percent_t value2 )
{
    size_t num_writ;
    rc_t rc = string_printf( pb->buffer, BUFFER_SIZE, &num_writ, fmt, value1, value2 );
    if ( rc == 0 )
        rc = write_buffer( pb, num_writ );
    return rc;
}

static rc_t progess_0a( progressbar * pb, const percent_t percent )
{
    return print_progress_1( pb, "| %2u%%", percent );
}


static rc_t progess_0( progressbar * pb, const percent_t percent )
{
    if ( percent & 1 )
        return print_progress_1( pb, "\b\b\b\b- %2u%%", percent );
    return print_progress_1( pb, "\b\b\b%2u%%", percent );
}


static rc_t progess_1a( progressbar * pb, const percent_t percent )
{
    percent_t p1 = percent / 10;
    percent_t p0 = percent - ( p1 * 10 );
    return print_progress_2( pb, "| %2u.%01u%%", p1, p0 );
}


static rc_t progess_1( progressbar * pb, const percent_t percent )
{
    percent_t p1 = percent / 10;
    percent_t p0 = percent - ( p1 * 10 );
    if ( ( p1 & 1 )&&( p0 == 0 ) )
        return print_progress_2( pb, "\b\b\b\b\b\b- %2u.%01u%%", p1, p0 );
    if (p1 != 100 || p0 != 0)
        return print_progress_2( pb, "\b\b\b\b\b%2u.%01u%%", p1, p0 );
    else
        return print_progress_2( pb, "\b\b\b\b\b%2u%%  \b\b",p1, p0);
}


static rc_t progess_2a( progressbar * pb, const percent_t percent )
{
    percent_t p1 = percent / 100;
    percent_t p0 = percent - ( p1 * 100 );
    return print_progress_2( pb, "| %2u.%02u%%", p1, p0 );
}


static rc_t progess_2( progressbar * pb, const percent_t percent )
{
    percent_t p1 = percent / 100;
    percent_t p0 = percent - ( p1 * 100 );
    if ( ( p1 & 1 )&&( p0 == 0 ) )
        return print_progress_2( pb, "\b\b\b\b\b\b\b- %2u.%02u%%", p1, p0 );
    if (p1 != 100 || p0 != 0)
        return print_progress_2( pb, "\b\b\b\b\b\b%2u.%02u%%", p1, p0 );
    else
        return print_progress_2( pb, "\b\b\b\b\b\b%2u%%   \b\b\b", p1, p0);
}

static rc_t progress_forward( progressbar * pb, const percent_t to )
{
    rc_t rc = 0;
    percent_t step = pb->percent;
    while ( rc == 0 && step < to )
    {
        step++;    
        switch( pb -> digits )
        {
            case 0 : rc = progess_0( pb, step ); break;
            case 1 : rc = progess_1( pb, step ); break;
            case 2 : rc = progess_2( pb, step ); break;
        }
    }
    pb->percent = to;
    return rc;
}

LIB_EXPORT rc_t CC update_progressbar( progressbar * pb, const percent_t percent )
{
    rc_t rc = 0;
    if ( pb == NULL )
        rc = RC( rcVDB, rcNoTarg, rcParsing, rcSelf, rcNull );
    else
    {
        percent_t to;
        switch( pb -> digits )
        {
            case 0 : to = percent > 100 ? 100 : percent; break;
            case 1 : to = percent > 1000 ? 1000 : percent; break;
            case 2 : to = percent > 10000 ? 10000 : percent; break;
        }
        
        if ( pb->initialized )
        {
            if ( to > pb->percent )
                rc = progress_forward( pb, to );
        }
        else
        {
            switch( pb -> digits )
            {
                case 0 : rc = progess_0a( pb, 0 ); break;
                case 1 : rc = progess_1a( pb, 0 ); break;
                case 2 : rc = progess_2a( pb, 0 ); break;
            }
            if ( rc == 0 )
            {
                pb->initialized = true;
                if ( to > 0 )
                    rc = progress_forward( pb, to );
            }
        }
    }
    return rc;
}
