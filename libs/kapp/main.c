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

#include <kapp/extern.h>
#include "main-priv.h"
#include <sysalloc.h>
#include <kapp/main.h>
#include <kfg/config.h>
#include <kproc/procmgr.h>
#include <klib/report.h>
#include <klib/writer.h>
#include <klib/log.h>
#include <klib/text.h>
#include <klib/status.h>
#include <klib/rc.h>
#include <kns/manager.h>

#include <strtol.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

rc_t KMane ( int argc, char *argv [] )
{
    rc_t rc = KMain(argc, argv);
#if _DEBUGGING
    if ( rc != 0 )
    {
        rc_t rc2;
        uint32_t lineno;
        const char *filename, *function;
        while ( GetUnreadRCInfo ( & rc2, & filename, & function, & lineno ) )
        {
            pLogErr ( klogWarn, rc2, "$(filename):$(lineno) within $(function)"
                     , "filename=%s,lineno=%u,function=%s"
                     , filename
                     , lineno
                     , function
                     );
        }
    }
#endif
    
    return rc;
}
