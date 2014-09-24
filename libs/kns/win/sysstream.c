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

/*--------------------------------------------------------------------------
 * forwards
 */
#define KSTREAM_IMPL KStdIOStream
typedef struct KStdIOStream KStdIOStream;

#include <kns/extern.h>
#include <kns/stream.h>
#include <kns/impl.h>
#include <klib/rc.h>
#include <klib/log.h>

#include <sysalloc.h>

#include "stream-priv.h"

#include <assert.h>

#include <os-native.h>

/*--------------------------------------------------------------------------
 * KStdIOStream
 *  a virtual stream
 */
struct KStdIOStream
{
    KStream dad;
    HANDLE fd;
};

static
rc_t CC KStdIOStreamWhack ( KStdIOStream *self )
{
    /* we don't close self->fd because we did not open it */
    free ( self );
    return 0;
}

static
rc_t CC KStdIOStreamRead ( const KStdIOStream *self,
    void *buffer, size_t bsize, size_t *num_read )
{
    DWORD to_read;

    assert ( self != NULL );

    to_read = ( DWORD ) bsize;
    if ( ( size_t ) to_read < bsize )
        to_read = -1;

    while ( 1 )
    {
        rc_t rc;
        DWORD count, lerrno;

        if ( ReadFile ( self -> fd, buffer, to_read, & count, NULL ) )
        {
            * num_read = count;
            break;
        }

        lerrno = GetLastError ();
        switch ( lerrno )
        {
        case ERROR_HANDLE_EOF:
            * num_read = 0;
            break;
        case ERROR_IO_PENDING:
            continue;
        default:
            rc = RC ( rcNS, rcStream, rcReading, rcNoObj, rcUnknown );
            PLOGERR ( klogErr,
                      ( klogErr, rc, "unknown system error '$(S)($(E))'",
                        "S=%!,E=%d", lerrno, lerrno ) );
            return rc;
        }
        break;
    }

    return 0;
}

static
rc_t CC KStdIOStreamWrite ( KStdIOStream *self,
    const void *buffer, size_t size, size_t *num_writ )
{
    DWORD to_write;

    assert ( self != NULL );

    to_write = ( DWORD ) size;
    if ( ( size_t ) to_write < size )
        to_write = -1;

    * num_writ = 0;

    while ( 1 )
    {
        rc_t rc;
        DWORD lerrno, count = 0;

        if ( WriteFile ( self -> fd, buffer, to_write, & count, NULL ) == 0 )
        {
            * num_writ += count;
            break;
        }

        lerrno = GetLastError ();
        switch ( lerrno )
        {
        case ERROR_IO_PENDING:
            if ( count != 0 )
            {
                buffer = & ( ( const char* ) buffer ) [ count ];
                to_write -= count;
                * num_writ += count;
                if ( to_write == 0 )
                    break;
            }
            Sleep ( 100 );
            continue;

        case ERROR_NOT_ENOUGH_MEMORY:
            rc = RC ( rcNS, rcStream, rcWriting, rcStorage, rcExhausted );
            LOGERR ( klogSys, rc, "system device full error" );
            return rc;

        default:
            rc = RC ( rcNS, rcStream, rcWriting, rcNoObj, rcUnknown );
            PLOGERR ( klogErr,
                      ( klogErr, rc, "unknown system error '$(S)($(E))'",
                        "S=%!,E=%d", lerrno, lerrno ) );
            return rc;
        }
        break;
    }

    return 0;
}

static KStream_vt_v1 vtKStdIOStream =
{
    1, 0,
    KStdIOStreamWhack,
    KStdIOStreamRead,
    KStdIOStreamWrite
};


static
rc_t KStdIOStreamMake ( KStream **sp, HANDLE fd, const char *strname,
    bool read_enabled, bool write_enabled )
{
    rc_t rc;

    if ( sp == NULL )
        rc = RC ( rcNS, rcStream, rcConstructing, rcParam, rcNull );
    else
    {
        KStdIOStream *s = calloc ( sizeof *s, 1 );
        if ( s == NULL )
            rc = RC ( rcNS, rcStream, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KStreamInit ( & s -> dad, ( const KStream_vt* ) & vtKStdIOStream,
                               "KStdIOStream", strname, read_enabled, write_enabled );
            if ( rc == 0 )
            {
                s -> fd = fd;
                * sp = & s -> dad;
                return 0;
            }

            free ( s );
        }

        * sp = NULL;
    }

    return rc;
}

/* MakeStdIn
 *  creates a read-only stream on stdin
 */
LIB_EXPORT rc_t CC KStreamMakeStdIn ( const KStream **std_in )
{
    HANDLE fd = GetStdHandle ( STD_INPUT_HANDLE );
    return KStdIOStreamMake ( ( KStream** ) std_in, fd, "stdin", true, false );
}

/* MakeStdOut
 * MakeStdErr
 *  creates a write-only stream on stdout or stderr
 */
LIB_EXPORT rc_t CC KStreamMakeStdOut ( KStream **std_out )
{
    HANDLE fd = GetStdHandle ( STD_OUTPUT_HANDLE );
    return KStdIOStreamMake ( std_out, fd, "stdout", false, true );
}

LIB_EXPORT rc_t CC KStreamMakeStdErr ( KStream **std_err )
{
    HANDLE fd = GetStdHandle ( STD_ERROR_HANDLE );
    return KStdIOStreamMake ( std_err, fd, "stderr", false, true );
}
