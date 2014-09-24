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
#include <klib/rc.h>
#include <klib/text.h>
#include "url-fetcher-impl.h"
#include <sysalloc.h>

#include <stdlib.h>

/* to define the latest version of the url-fetcher in the vt */
#define KURLFETCHER_LATEST 1

static const char classname[] = "KUrlFetcher";

/* AddRef
 *  creates a new reference
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KUrlFetcherAddRef ( const KUrlFetcher *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, classname ) )
        {
        case krefLimit:
            return RC ( rcApp, rcFunction, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

/* Release
 *  discard reference to list
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KUrlFetcherRelease ( const KUrlFetcher *cself )
{
    KUrlFetcher *self = ( KUrlFetcher* ) cself;
    if ( cself != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, classname ) )
        {
        case krefWhack:
            {
                rc_t rc;
                switch ( self -> vt -> v1 . maj )
                {
                case 1:
                    rc = ( * self -> vt -> v1 . destroy ) ( self );
                    break;

                default:
                    rc = RC ( rcApp, rcFunction, rcReleasing, rcInterface, rcBadVersion );
                }

                if ( rc != 0 )
                    KRefcountInit ( & self -> refcount, 1, classname, "failed-release", "orphan" );
                return rc;
            }
        case krefLimit:
            return RC ( rcApp, rcFunction, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Read
 *  reads data from the internal buffer ...
 */
LIB_EXPORT rc_t CC KUrlFetcherRead( KUrlFetcher *self, const char * uri,
                      void *dst, size_t to_read, size_t *num_read )
{
    if ( self == NULL )
        return RC ( rcApp, rcFunction, rcAccessing, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . read ) ( self, uri, dst, to_read, num_read );
    }

    return RC ( rcApp, rcFunction, rcAccessing, rcInterface, rcBadVersion );
}


/* Init
 *  polymorphic parent constructor
 */
LIB_EXPORT rc_t CC KUrlFetcherInit ( KUrlFetcher *self, const KUrlFetcher_vt *vt )
{
    if ( self == NULL || vt == NULL )
        return RC ( rcApp, rcFunction, rcConstructing, rcSelf, rcNull );

    if ( vt -> v1 . maj == 0 )
        return RC ( rcApp, rcFunction, rcConstructing, rcInterface, rcInvalid );

    if ( vt -> v1 . maj > KURLFETCHER_LATEST )
        return RC ( rcApp, rcFunction, rcConstructing, rcInterface, rcBadVersion );

    self -> vt = vt;
    KRefcountInit ( & self -> refcount, 1, classname, "initialized", "regular" );
    return 0;
}
