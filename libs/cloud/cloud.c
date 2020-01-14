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

#include <cloud/extern.h>
#include <cloud/impl.h>

#include <klib/rc.h>
#include <klib/status.h>
#include <kns/manager.h>
#include <kns/stream.h>

#include <assert.h>

/*--------------------------------------------------------------------------
 * Cloud
 */

/* Destroy
 */
static rc_t CloudDestroy ( Cloud * self )
{
    if ( self == NULL )
        return RC ( rcCloud, rcProvider, rcAccessing, rcSelf, rcNull );

    switch ( self -> vt -> v1 . maj )
    {
    case 1:
        return ( * self -> vt -> v1 . destroy ) ( self );
    }

    return RC ( rcCloud, rcProvider, rcAccessing, rcInterface, rcBadVersion );
}

/* AddRef
 * Release
 */
LIB_EXPORT rc_t CC CloudAddRef ( const Cloud * self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "Cloud" ) )
        {
        case krefLimit:
            return RC ( rcCloud, rcProvider, rcAttaching, rcRange, rcExcessive );
        case krefNegative:
            return RC ( rcCloud, rcProvider, rcAttaching, rcSelf, rcInvalid );
        default:
            break;
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC CloudRelease ( const Cloud * self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "Cloud" ) )
        {
        case krefWhack:
            return CloudDestroy ( ( Cloud * ) self );
        case krefNegative:
            return RC ( rcCloud, rcProvider, rcReleasing, rcRange, rcExcessive );
        default:
            break;
        }
    }

    return 0;
}

/* MakeComputeEnvironmentToken
 *  contact cloud provider to get proof of execution environment in form of a token
 */
LIB_EXPORT rc_t CC CloudMakeComputeEnvironmentToken ( const Cloud * self, struct String const ** ce_token )
{
    rc_t rc;

    if ( ce_token == NULL )
        rc = RC ( rcCloud, rcProvider, rcAccessing, rcParam, rcNull );
    else
    {

        * ce_token = NULL;

        if ( self == NULL )
            rc = RC ( rcCloud, rcProvider, rcAccessing, rcSelf, rcNull );
        else
        {
            switch ( self -> vt -> v1 . maj )
            {
            case 1:
                return ( * self -> vt -> v1 . make_cet ) ( self, ce_token );
            }

            rc = RC ( rcCloud, rcProvider, rcAccessing, rcInterface, rcBadVersion );
        }
    }

    return rc;
}

/* GetLocation
 *  get cloud location in form proovider.zone
 */
LIB_EXPORT rc_t CC CloudGetLocation( const Cloud * self,
    struct String const ** location )
{
    rc_t rc;

    if (location == NULL )
        rc = RC ( rcCloud, rcProvider, rcAccessing, rcParam, rcNull );
    else
    {

        *location = NULL;

        if ( self == NULL )
            rc = RC ( rcCloud, rcProvider, rcAccessing, rcSelf, rcNull );
        else
        {
            switch ( self -> vt -> v1 . maj )
            {
            case 1:
                return (*self -> vt -> v1 . get_location) (self, location);
            }

            rc = RC ( rcCloud, rcProvider, rcAccessing, rcInterface, rcBadVersion );
        }
    }

    return rc;
}

/* AddComputeEnvironmentTokenForSigner
 *  prepare a request object with a compute environment token
 *  for use by an SDL-associated "signer" service
 */
LIB_EXPORT rc_t CC CloudAddComputeEnvironmentTokenForSigner ( const Cloud * self, struct KClientHttpRequest * req )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcCloud, rcProvider, rcAccessing, rcSelf, rcNull );
    else if ( req == NULL )
        rc = RC ( rcCloud, rcProvider, rcAccessing, rcParam, rcNull );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . add_cet_to_req ) ( self, req );
        }

        rc = RC ( rcCloud, rcProvider, rcAccessing, rcInterface, rcBadVersion );
    }

    return rc;
}

/* AddAuthentication
 *  prepare a request object with credentials for user-pays
 */
LIB_EXPORT rc_t CC CloudAddAuthentication ( const Cloud * self,
    struct KClientHttpRequest * req, const char * http_method )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcCloud, rcProvider, rcAccessing, rcSelf, rcNull );
    else if ( req == NULL || http_method == NULL )
        rc = RC ( rcCloud, rcProvider, rcAccessing, rcParam, rcNull );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . add_authn ) ( self, req, http_method );
        }

        rc = RC ( rcCloud, rcProvider, rcAccessing, rcInterface, rcBadVersion );
    }

    return rc;
}

/* AddUserPaysCredentials
 *  prepare a request object with credentials for user-pays
 */
LIB_EXPORT rc_t CC CloudAddUserPaysCredentials ( const Cloud * self,
    struct KClientHttpRequest * req, const char * http_method )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcCloud, rcProvider, rcAccessing, rcSelf, rcNull );
    else if ( req == NULL || http_method == NULL )
        rc = RC ( rcCloud, rcProvider, rcAccessing, rcParam, rcNull );
    else
    {
        switch ( self -> vt -> v1 . maj )
        {
        case 1:
            return ( * self -> vt -> v1 . add_user_pays_to_req ) ( self, req, http_method );
        }

        rc = RC ( rcCloud, rcProvider, rcAccessing, rcInterface, rcBadVersion );
    }

    return rc;
}

/* Init
 *  initialize a newly allocated cloud object
 */
LIB_EXPORT rc_t CC CloudInit ( Cloud * self, const Cloud_vt * vt,
    const char * classname, const KNSManager * kns, bool user_agrees_to_pay,
    bool user_agrees_to_reveal_instance_identity )
{
    rc_t rc = 0;

    if ( self == NULL )
        return RC ( rcCloud, rcProvider, rcConstructing, rcSelf, rcNull );

    if ( vt == NULL )
        return RC ( rcCloud, rcProvider, rcConstructing, rcInterface, rcNull );

    if ( kns == NULL )
        return RC ( rcCloud, rcProvider, rcConstructing, rcParam, rcNull );

    switch ( vt -> v1 . maj )
    {
    case 0:
        return RC ( rcCloud, rcProvider, rcConstructing, rcInterface, rcInvalid );

    case 1:
        switch ( vt -> v1 . min )
        {
        case 0:
#if _DEBUGGING
            if ( vt -> v1 . add_user_pays_to_req == NULL ||
                 vt -> v1 . add_authn == NULL            ||
                 vt -> v1 . add_cet_to_req == NULL       ||
                 vt -> v1 . get_location == NULL        ||
                 vt -> v1 . make_cet == NULL             ||
                 vt -> v1 . destroy == NULL              )
                return RC ( rcCloud, rcProvider, rcConstructing, rcInterface, rcNull );
#endif
            break;
        default:
            return RC ( rcCloud, rcProvider, rcConstructing, rcInterface, rcBadVersion );
        }
        break;

    default:
        return RC ( rcCloud, rcProvider, rcConstructing, rcInterface, rcBadVersion );
    }

    rc = KNSManagerAddRef ( kns );
    if ( rc == 0 )
    {
        self -> vt = vt;
        self -> kns = kns;
        self -> user_agrees_to_pay = user_agrees_to_pay;
        self -> user_agrees_to_reveal_instance_identity
            = user_agrees_to_reveal_instance_identity;
        KRefcountInit ( & self -> refcount, 1, classname, "init", "" );
    }

    return rc;
}

/* Whack
 *  run destructor and free object
 */
LIB_EXPORT rc_t CC CloudWhack ( Cloud * self )
{
    if ( self != NULL )
    {
        KNSManagerRelease ( self -> kns );
        KStreamRelease( self -> conn );

        free ( self );
    }
    return 0;
}

/* Set a pre-opened HTTP connection, for testing (NULL OK)
 * TODO: remove when mocked connection becomes a regular feature of KNS
 */
LIB_EXPORT rc_t CC CloudSetHttpConnection ( Cloud  * self, struct KStream * conn )
{
    if ( self != NULL )
    {
        if ( self -> conn != NULL )
        {
            KStreamRelease( self -> conn );
        }
        self -> conn = conn;
        if ( self -> conn != NULL )
        {
            KStreamAddRef ( self -> conn );
        }
    }
    return 0;
}

void CloudSetUserAgreesToRevealInstanceIdentity(Cloud  * self,
    bool value)
{
    if (self != NULL)
        self->user_agrees_to_reveal_instance_identity = value;
}
