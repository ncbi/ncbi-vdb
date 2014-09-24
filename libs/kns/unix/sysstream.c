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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KStdIOStream
 *  a virtual stream
 */
struct KStdIOStream
{
    KStream dad;
    int fd;
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
    rc_t rc;
    assert ( self != NULL );

    while ( 1 )
    {
        int lerrno;

        ssize_t count = read ( self -> fd, buffer, bsize );

        if ( count < 0 ) switch ( lerrno = errno )
        {
        case ENOSPC:
            rc = RC ( rcNS, rcStream, rcReading, rcStorage, rcExhausted );
            LOGERR (klogSys, rc, "system device full error");
            return rc;

        case EINTR:
            continue;

        case EFBIG:
            rc = RC ( rcNS, rcStream, rcReading, rcStream, rcExcessive );
            LOGERR (klogErr, rc, "system file too big error");
            return rc;

        case EIO:
            rc = RC ( rcNS, rcStream, rcReading, rcTransfer, rcUnknown );
            LOGERR (klogErr, rc, "system I/O error - broken pipe");
            return rc;
            
        case EBADF:
            rc = RC ( rcNS, rcStream, rcReading, rcFileDesc, rcInvalid );
            PLOGERR (klogInt,
                     (klogInt, rc, "system bad file descriptor error fd=$(F)",
                      "F=%d", lerrno, self->fd));
            return rc;

        case EINVAL:
            rc = RC ( rcNS, rcStream, rcReading, rcParam, rcInvalid );
            LOGERR (klogInt, rc, "system invalid argument error");
            return rc;

        default:
            rc = RC ( rcNS, rcStream, rcReading, rcNoObj, rcUnknown );
            PLOGERR (klogErr,
                     (klogErr, rc, "unknown system error '$(S)($(E))'",
                      "S=%!,E=%d", lerrno, lerrno));
            return rc;
        }

        assert ( num_read != NULL );
        * num_read = count;
        break;
    }

    return 0;
}

static
rc_t CC KStdIOStreamWrite ( KStdIOStream *self,
    const void *buffer, size_t size, size_t *num_writ )
{
    rc_t rc;
    assert ( self != NULL );

    while ( 1 )
    {
        int lerrno;

        ssize_t count = write ( self -> fd, buffer, size );

        if ( count < 0 ) switch ( lerrno = errno )
        {
        case ENOSPC:
            rc = RC ( rcNS, rcStream, rcWriting, rcStorage, rcExhausted );
            LOGERR (klogSys, rc, "system device full error");
            return rc;

        case EINTR:
            continue;

        case EFBIG:
            rc = RC ( rcNS, rcStream, rcWriting, rcStream, rcExcessive );
            LOGERR (klogErr, rc, "system file too big error");
            return rc;

        case EIO:
            rc = RC ( rcNS, rcStream, rcWriting, rcTransfer, rcUnknown );
            LOGERR (klogErr, rc, "system I/O error - broken pipe");
            return rc;
            
        case EBADF:
            rc = RC ( rcNS, rcStream, rcWriting, rcFileDesc, rcInvalid );
            PLOGERR (klogInt,
                     (klogInt, rc, "system bad file descriptor error fd=$(F)",
                      "F=%d", lerrno, self->fd));
            return rc;

        case EINVAL:
            rc = RC ( rcNS, rcStream, rcWriting, rcParam, rcInvalid );
            LOGERR (klogInt, rc, "system invalid argument error");
            return rc;

        default:
            rc = RC ( rcNS, rcStream, rcWriting, rcNoObj, rcUnknown );
            PLOGERR (klogErr,
                     (klogErr, rc, "unknown system error '$(S)($(E))'",
                      "S=%!,E=%d", lerrno, lerrno));
            return rc;
        }

        assert ( num_writ != NULL );
        * num_writ = count;
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
rc_t KStdIOStreamMake ( KStream **sp, int fd, const char *strname,
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
    return KStdIOStreamMake ( ( KStream** ) std_in, 0, "stdin", true, false );
}

/* MakeStdOut
 * MakeStdErr
 *  creates a write-only stream on stdout or stderr
 */
LIB_EXPORT rc_t CC KStreamMakeStdOut ( KStream **std_out )
{
    return KStdIOStreamMake ( std_out, 1, "stdout", false, true );
}

LIB_EXPORT rc_t CC KStreamMakeStdErr ( KStream **std_err )
{
    return KStdIOStreamMake ( std_err, 2, "stderr", false, true );
}
