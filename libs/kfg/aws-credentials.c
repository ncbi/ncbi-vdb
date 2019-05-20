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


#include <kfg/aws-credentials.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <klib/defs.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/text.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIB_EXPORT rc_t CC LoadAwsCredentialsFromFile ( const char *profile,
    char **aws_access_key_id, char **aws_secret_access_key )
{
    rc_t rc = 0;

    if ( aws_access_key_id == NULL || aws_secret_access_key == NULL ) {
        rc = RC ( rcKFG, rcAttr, rcListing, rcParam, rcNull );
        return rc;
    }


    return rc;
}

LIB_EXPORT rc_t CC LoadAwsCredentialsFromEnv (
    char **aws_access_key_id, char **aws_secret_access_key )
{
    rc_t rc = 0;

    if ( aws_access_key_id == NULL || aws_secret_access_key == NULL ) {
        rc = RC ( rcKFG, rcAttr, rcListing, rcParam, rcNull );
        return rc;
    }

    *aws_access_key_id = getenv ( "AWS_ACCESS_KEY_ID" );
    *aws_secret_access_key = getenv ( "AWS_SECRET_ACCESS_KEY" );
    if ( *aws_access_key_id == NULL || *aws_secret_access_key == NULL
        || strlen ( *aws_access_key_id ) < 1
        || strlen ( *aws_secret_access_key ) < 1 ) {
        rc = RC ( rcKFG, rcAttr, rcListing, rcEnvironment, rcNotAvailable );
        *aws_access_key_id = NULL;
        *aws_secret_access_key = NULL;
        return rc;
    }

    *aws_access_key_id = strdup ( *aws_access_key_id );
    *aws_secret_access_key = strdup ( *aws_secret_access_key );

    return rc;
}
