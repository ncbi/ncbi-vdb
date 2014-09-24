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

#include <vdb/extern.h>
#include "schema-tok.h"
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * KToken
 */


rc_t KTokenRCExplain ( const KToken *self, KLogLevel lvl, rc_t rc )
{
    if ( rc != 0 )
    {
        PLOGERR (lvl, ( lvl, rc, "$(file):$(lineno)", "file=%.*s,lineno=%u"
                   , ( int ) self -> txt -> path . size, self -> txt -> path . addr
                   , self -> lineno ));
    }
    return rc;
}

rc_t KTokenFailure ( const KToken *self, KLogLevel lvl, rc_t rc, const char *expected )
{
    if ( GetRCState ( rc ) != rcUnexpected )
        return KTokenRCExplain ( self, lvl, rc );

    PLOGMSG ( lvl, ( lvl, "$(file):$(lineno): "
               "expected '$(expected)' but found '$(found)'",
               "file=%.*s,lineno=%u,expected=%s,found=%.*s"
               , ( int ) self -> txt -> path . size, self -> txt -> path . addr
               , self -> lineno
               , expected
               , ( int ) self -> str . size, self -> str . addr ));

    return rc;
}

rc_t KTokenExpected ( const KToken *self, KLogLevel lvl, const char *expected )
{
    return KTokenFailure ( self, lvl,
        RC ( rcVDB, rcSchema, rcParsing, rcToken, rcUnexpected ), expected );
}
