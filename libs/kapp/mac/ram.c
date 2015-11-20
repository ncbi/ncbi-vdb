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

#include "../main-priv.h"
#include <kapp/main.h>
#include <klib/log.h>
#include <klib/rc.h>

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/sysctl.h>

/* KAppGetTotalRam
 *  Mac specific function of getting amount of RAM
 */
rc_t KAppGetTotalRam ( uint64_t * totalRam )
{
    rc_t rc;

    size_t len = sizeof *totalRam;
    int ret;

    assert ( totalRam != 0 );

    ret = sysctlbyname("hw.memsize", totalRam, &len, NULL, 0 );
    if ( ret < 0 )
    {
		int status = errno;
        rc = RC ( rcApp, rcNoTarg, rcInitializing, rcMemory, rcFailed );
        PLOGERR ( klogFatal, ( klogFatal, rc,
                    "failed to retrieve number of RAM pages. error code: $(status) - $(msg)"
                    , "status=%d,msg=%!"
                    , status, status
                        ));
        return rc;
    }

    return 0;
}
