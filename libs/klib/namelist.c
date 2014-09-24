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
#include <klib/impl.h>
#include <sysalloc.h>

#define KNAMELIST_LATEST 1

/*--------------------------------------------------------------------------
 * KNamelist
 *  a simple array of entry names
 */

/* AddRef
 *  creates a new reference
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KNamelistAddRef ( const KNamelist *self )
{
    if ( self != NULL )
        atomic_inc ( & ( ( KNamelist* ) self ) -> refcount );
    return 0;
}

/* Release
 *  discard reference to list
 *  ignores NULL references
 */
LIB_EXPORT rc_t CC KNamelistRelease ( const KNamelist *cself )
{
    KNamelist *self = ( KNamelist* ) cself;
    if ( cself != NULL )
    {
        if ( atomic_dec_and_test ( & self -> refcount ) )
        {
            rc_t status;
            switch ( self -> vt -> v1 . maj )
            {
            case 1:
                status = ( * self -> vt -> v1 . destroy ) ( self );
                break;
            default:
                status = RC ( rcCont, rcNamelist, rcReleasing, rcInterface, rcBadVersion );
            }

            if ( status != 0 )
                atomic_set ( & self -> refcount, 1 );

            return status;
        }
    }
    return 0;
}

/* Count
 *  returns the number of entries
 *
 *  "count" [ OUT ] - return parameter for entry count
 */
LIB_EXPORT rc_t CC KNamelistCount ( const KNamelist *self, uint32_t *count )
{
    if ( count == NULL )
        return RC ( rcCont, rcNamelist, rcAccessing, rcParam, rcNull );

    * count = 0;

    if ( self == NULL )
        return RC ( rcCont, rcNamelist, rcAccessing, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . count ) ( self, count );
    }

    return RC ( rcCont, rcNamelist, rcAccessing, rcInterface, rcBadVersion );
}

/* Get
 *  get an indexed name
 *
 *  "idx" [ IN ] - a zero-based name index
 *
 *  "name" [ OUT ] - return parameter for NUL terminated name
 */
LIB_EXPORT rc_t CC KNamelistGet ( const KNamelist *self,
    uint32_t idx, const char **name )
{
    if ( name == NULL )
        return RC ( rcCont, rcNamelist, rcAccessing, rcParam, rcNull );

    * name = NULL;

    if ( self == NULL )
        return RC ( rcCont, rcNamelist, rcAccessing, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . get ) ( self, idx, name );
    }

    return RC ( rcCont, rcNamelist, rcAccessing, rcInterface, rcBadVersion );
}

/* Init
 *  polymorphic parent constructor
 */
LIB_EXPORT rc_t CC KNamelistInit ( KNamelist *self, const KNamelist_vt *vt )
{
    if ( self == NULL || vt == NULL )
        return RC ( rcCont, rcNamelist, rcConstructing, rcSelf, rcNull );

    if ( vt -> v1 . maj == 0 )
        return RC ( rcCont, rcNamelist, rcConstructing, rcInterface, rcInvalid );

    if ( vt -> v1 . maj > KNAMELIST_LATEST )
        return RC ( rcCont, rcNamelist, rcConstructing, rcInterface, rcBadVersion );

    self -> vt = vt;
    atomic_set ( & self -> refcount, 1 );

    return 0;
}
