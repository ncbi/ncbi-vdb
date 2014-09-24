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
#include <align/extern.h>

#include <klib/rc.h>
#include <klib/container.h>
#include <klib/refcount.h>
#include <klib/sort.h>
#include <insdc/insdc.h>
#include <align/iterator.h>
#include <align/manager.h>
#include <sysalloc.h>

#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


struct AlignMgr
{
    KRefcount refcount;
    /* what else will be here ? */
};


LIB_EXPORT rc_t CC AlignMgrMakeRead ( const struct AlignMgr **self )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcConstructing, rcSelf, rcNull );
    else
    {
        AlignMgr * alm = calloc( sizeof * alm, 1 );
        if ( alm == NULL )
            rc = RC( rcAlign, rcIterator, rcConstructing, rcMemory, rcExhausted );
        else
        {
            KRefcountInit( &alm->refcount, 1, "AlignMgr", "Make", "align" );
        }

        if ( rc == 0 )
            *self = alm;
        else
            free( alm );
    }
    return rc;

}


LIB_EXPORT rc_t CC AlignMgrAddRef ( const AlignMgr *self )
{
    rc_t rc = 0;
    if ( self == NULL )
        rc = RC( rcAlign, rcIterator, rcAttaching, rcSelf, rcNull );
    else
    {
        if ( KRefcountAdd( &self->refcount, "AlignMgr" ) != krefOkay )
        {
            rc = RC( rcAlign, rcIterator, rcAttaching, rcError, rcUnexpected );
        }
    }
    return rc;
}


LIB_EXPORT rc_t CC AlignMgrRelease ( const AlignMgr *cself )
{
    rc_t rc = 0;
    if ( cself == NULL )
        rc = RC( rcAlign, rcIterator, rcReleasing, rcSelf, rcNull );
    else
    {
        if ( KRefcountDrop( &cself->refcount, "AlignMgr" ) == krefWhack )
        {
            AlignMgr * self = ( AlignMgr * ) cself;
            free( self );
        }
    }
    return rc;

}
