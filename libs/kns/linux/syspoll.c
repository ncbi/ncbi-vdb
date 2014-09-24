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


#include <kns/extern.h>
#include <klib/log.h>
#include <klib/out.h>
#include <kproc/timeout.h>
#include <os-native.h>

#include "poll-priv.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <os-native.h>

#include <poll.h>
#include <unistd.h>
#include <errno.h>


/* socket_wait
 *  wait for an event or a timeout
 */
int socket_wait ( int fd, int events, timeout_t *tm )
{
    int i, status;
    struct pollfd fds [ 1 ];
        
    /* poll for data with no delay */
    for ( i = 0; i < 2; ++ i )
    {
        fds [ 0 ] . fd = fd;
        fds [ 0 ] . events = events;
        fds [ 0 ] . revents = 0;
        
        status = poll ( fds, sizeof fds / sizeof fds [ 0 ], 0 );
        if ( status > 0 )
            return fds [ 0 ] . revents;
        if ( status < 0 )
            return -1;
    }

    /* test for infinite timeout */
    while ( tm == NULL )
    {
        status = poll ( fds, sizeof fds / sizeof fds [ 0 ], 1000 );
        if ( status > 0 )
            return fds [ 0 ] . revents;
        if ( status < 0 )
            return -1;

        /* TBD - check for quitting event
           this may want to be a callback on the manager
           or on the socket */
    }

    /* no blocking */
    if ( tm -> mS == 0 )
        return 0;

#if PPOLL_USED_A_PROPER_TIMEOUT_INSTEAD_OF_BEING_CLOWNS

    /* convert to absolute time */
    if ( ! tm -> prepared )
        TimeoutPrepare ( tm );

    /* wait until final guy */
    status = ppoll ( fds, sizeof fds / sizeof fds [ 0 ], & tm -> ts, NULL );

#elif WHY_BOTHER_WITH_PPOLL_WHEN_POLL_TAKES_A_RELATIVE_TIMEOUT_ALREADY || 1

    /* wait until final guy */
    status = poll ( fds, sizeof fds / sizeof fds [ 0 ], tm -> mS );

#endif

    if ( status > 0 )
        return fds [ 0 ] . revents;

    return status;
}
