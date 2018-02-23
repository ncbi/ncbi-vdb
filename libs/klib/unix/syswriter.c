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
#include <sysalloc.h>
#include "writer-priv.h"
#include <klib/text.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static int unix_stdout = 1;
static int unix_stderr = 2;

rc_t KWrtSysInit(void** h_stdout, void** h_stderr)
{
    if( h_stdout ) {
        *h_stdout = &unix_stdout;
    }
    if( h_stderr ) {
        *h_stderr = &unix_stderr;
    }
    return 0;
}

/* ----
 * write 'count' bytes starting at 'buf'  to a "stream/file" identified by 'fd'
 */
rc_t CC KWrt_DefaultWriter ( void * self, const char * buffer, size_t bufsize, size_t * num_writ )
{
    size_t total;
    size_t remaining;
    ssize_t num_written;
    const char * pbyte;
    int handle;
    rc_t rc;

    if ( self == NULL ) {
        return RC ( rcApp, rcLog, rcWriting, rcSelf, rcNull );
    }

    handle = *(int*)self;
    pbyte = buffer;
    rc = 0;

    total = 0;
    for (remaining = bufsize; remaining > 0; remaining -= num_written)
    {
        num_written = write (handle, pbyte, remaining);
        if (num_written < 0)
        {
            switch (errno)
            {
            case ENOSPC:
                rc = RC ( rcApp, rcLog, rcWriting, rcStorage, rcExhausted );
                break;
            case EINTR:
                num_written = 0;
                break;
            case EFBIG:
                rc = RC ( rcApp, rcLog, rcWriting, rcFile, rcExcessive );
                break;
            case EIO:
                rc = RC ( rcApp, rcLog, rcWriting, rcTransfer, rcUnknown );
                break;
            case EBADF:
                rc = RC ( rcApp, rcLog, rcWriting, rcFileDesc, rcInvalid );
                break;
            case EINVAL:
                rc = RC ( rcApp, rcLog, rcWriting, rcParam, rcInvalid );
                break;
            default:
                rc = RC ( rcApp, rcLog, rcWriting, rcNoObj, rcUnknown );
                break;
            }
            if (rc)
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
    if (*len > 3)
    {
        if ((fmt[*len-3] == 'l') &&
            (fmt[*len-2] == 'l'))
        {
            --*len;
            fmt[*len-2] = 'j';
            fmt[*len-1] = fmt[*len];
            fmt[*len] = '\0';
        }
    }
    else if (*len > 2)
    {
        /* -----
         * replace an 'l' with a 'j'
         * on 32 bit l is 32 bits and j is 64 bits
         * on 64 bit l is 64 bits and j is 64 bits
         * so we want the one that is the same on both
         * we are not otherwise "fixing" the format for bad specifications
         * just this ambiguous specification.
         */
        if (fmt[*len-2] == 'l')
            fmt[*len-2] = 'j';
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
    /* from <unistd.h> */
    return write( fd, buf, count );
}

int CC sys_is_a_tty( int fd )
{
    return isatty( fd );
}
