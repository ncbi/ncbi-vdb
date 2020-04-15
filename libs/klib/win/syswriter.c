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
#include <klib/writer.h>
#include <klib/text.h>
#include <klib/printf.h>
#include "writer-priv.h"

#include <sysalloc.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <io.h>

#include <os-native.h>

static HANDLE win_stdout;
static HANDLE win_stderr;

rc_t KWrtSysInit(void** h_stdout, void** h_stderr)
{
    win_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    win_stderr = GetStdHandle(STD_ERROR_HANDLE);

    if( h_stdout ) {
        *h_stdout = &win_stdout;
    }
    if( h_stderr ) {
        *h_stderr = &win_stderr;
    }
    return 0;
}

#if _MSC_VER < 1900
KLIB_EXTERN int CC snprintf(char * buffer, size_t bufsize, const char * format, ...)
{
    int ret;
    size_t size;
    rc_t rc;
    va_list args;

    va_start (args, format);
    rc = string_vprintf ( buffer, bufsize, &size, format, args );
    if ( rc == 0 )
    {
        ret = ( int ) size;
    }
    else
    {
        if ( ( GetRCState( rc ) == rcInsufficient )&&( GetRCObject( rc ) == rcBuffer ) )
            ret = ( int ) size;
        else
            ret = -1;
    }
/*    ret = _vsnprintf (buffer, bufsize, format, args); */
    va_end (args);
    return ret;
}
#endif

/* ----
 * write 'count' bytes starting at 'buf'  to a "stream/file" identified by 'fd'
 */
rc_t CC KWrt_DefaultWriter( void * self, const char * buffer, size_t bufsize, size_t * num_writ )
{
    size_t total;
    DWORD remaining;
    int num_written;
    const char * pbyte;
    HANDLE handle;
    rc_t rc;

    if ( self == NULL ) {
        return RC ( rcApp, rcLog, rcWriting, rcSelf, rcNull );
    }

    handle = *(HANDLE *)self;
    pbyte = buffer;
    rc = 0;

    total = 0;
    for (remaining = ( DWORD ) bufsize; remaining > 0; remaining -= num_written)
    {
        if (WriteFile (handle, pbyte, remaining, &num_written, NULL) == 0)
        {       
            rc = RC ( rcApp, rcLog, rcWriting, rcTransfer, rcUnknown );
            break;
        }
        else
            total += num_written;
    }
    *num_writ = total;
    return rc;
}

void print_int_fixup ( char * fmt, size_t * len, size_t max )
{
    char type;
    /* all the Unix flavors support a real printf
     * Only Windows does it "wrong".
     * In windows we can't just return like this but instead:
     *    save fmt[len-1]
     *    replace 'j' with I64
     *    replace 'z' or 't',  with 'I'
     *    replace 'hh' with 'h'
     *    append saved fmt[len-1] to its new place.
     *
     */
    if (*len > 2)
    {
        if ((fmt [*len-3] == 'l') && (fmt [*len-2] == 'l'))
        {
            fmt [*len-2] = fmt [*len-1];
            fmt [*len-1] = '\0';
            --*len;
        }
    }
    if (*len > 2)
    {
        switch (fmt[*len-2])
        {
        case 'h':
            if (fmt[*len-3] == 'h')
            {
                fmt [*len-2] = fmt [*len-1];
                fmt [*len-1] = '\0';
                *len --;
            }
            break;

        case 'z': /* 32-bit specific */
        case 't':
            fmt[*len-2] = 'l';
            break;
        case 'j':
        case 'l':
            type = fmt[*len-1];
            fmt[*len-2] = 'I';
            fmt[*len-1] = '6';
            fmt[*len]   = '4';
            fmt[*len+1] = type;
            fmt[*len+2] = '\0';
            *len += 2;
            break;
        }
    }
    return;
}

void print_float_fixup ( char * fmt, size_t * len, size_t max )
{
    /* all the Unix flavors support a real printf
     * Only Windows does it "wrong".
     */
    return;
}

void print_char_fixup ( char * fmt, size_t * len, size_t max )
{
    /* all the Unix flavors support a real printf
     * Only Windows does it "wrong".
     */
    return;
}

size_t CC sys_simple_write( int fd, const void * buf, size_t count )
{
    /* from <io.h> */
    return write( fd, buf, count );
}

int CC sys_is_a_tty( int fd )
{
    /* from <io.h> */
    return _isatty( _fileno( fd ) );
}
