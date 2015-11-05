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

#define UNICODE 1
#define _UNICODE 1

#include "../main-priv.h"
#include <klib/log.h>
#include <klib/rc.h>

#include <WINDOWS.H>
#include <assert.h>

/* KAppGetTotalRam
 *  Windows specific function of getting amount of RAM
 */
rc_t KAppGetTotalRam ( uint64_t * totalRam )
{
    rc_t rc;
    BOOL ret;

    assert ( totalRam != 0 );

    ret = GetPhysicallyInstalledSystemMemory ( totalRam );
    if ( ! ret )
    {
		DWORD error = GetLastError();
        rc = RC ( rcApp, rcNoTarg, rcInitializing, rcMemory, rcFailed );
        PLOGERR ( klogFatal, ( klogFatal, rc,
                    "failed to retrieve size of RAM. error code: $(ERR_CODE) - $(ERR_MESSAGE)"
                    , "ERR_CODE=%u,ERR_MESSAGE=%!"
                    , (uint32_t)error, (uint32_t)error ));
        return rc;
    }

    *totalRam *= 1024;

    return 0;
}
