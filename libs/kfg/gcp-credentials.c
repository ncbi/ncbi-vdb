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


#include <kfg/gcp-credentials.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <klib/defs.h>
#include <klib/printf.h>
#include <klib/rc.h>
#include <klib/text.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


LIB_EXPORT
rc_t CC LoadGcpCredentials (
    const KFile *pathToJsonFile, KJsonValue **GcpAccountCredentials )
{
    rc_t rc = 0;
    KDirectory *dir = NULL;
    uint64_t json_size;
    char *buffer = NULL;
    *GcpAccountCredentials = NULL;

    rc = KDirectoryNativeDir ( &dir );
    if ( rc ) return rc;

    rc = KFileSize ( pathToJsonFile, &json_size );
    if ( rc ) return rc;

    buffer = (char *)calloc ( json_size + 1, 1 );

    rc = KFileReadExactly ( pathToJsonFile, 0, buffer, json_size );
    if ( rc ) {
        free ( buffer );
        return rc;
    }

    KJsonValue *root = NULL;
    rc = KJsonValueMake ( &root, buffer, NULL, 0 );
    if ( rc ) return rc;

    free ( buffer );

    const KJsonObject *obj = KJsonValueToObject ( root );

    const char *required[] = {"type", "project_id", "private_key_id",
        "private_key", "client_email", "client_id", "auth_uri", "token_uri",
        "auth_provider_x509_cert_url", "client_x509_cert_url", NULL};

    size_t i = 0;
    while ( required[i] != NULL ) {
        const KJsonValue *v = KJsonObjectGetMember ( obj, required[i] );
        if ( v == NULL ) {
            return RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
        }
        if ( KJsonGetValueType ( v ) != jsString ) {
            return RC ( rcKFG, rcFile, rcParsing, rcParam, rcInvalid );
        }
        ++i;
    }

    *GcpAccountCredentials = root;

    return rc;
}
