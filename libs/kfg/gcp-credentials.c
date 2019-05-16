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
#include <stdlib.h>


LIB_EXPORT
rc_t CC LoadGcpCredentials (
    const KFile *pathToJsonFile, KJsonValue **GcpAccountCredentials )
{
    rc_t rc = 0;
    KDirectory *dir = NULL;
    uint64_t json_size;
    char *buffer = NULL;

    rc = KDirectoryNativeDir ( &dir );
    if ( rc ) return rc;

    rc = KFileSize ( pathToJsonFile, &json_size );
    if ( rc ) return rc;

    buffer = (char *)malloc ( json_size );

    rc = KFileReadExactly ( pathToJsonFile, 0, buffer, json_size );
    if ( rc ) {
        free ( buffer );
        return rc;
    }

    rc = KJsonValueMake ( GcpAccountCredentials, buffer, NULL, 0 );
    free ( buffer );

    return rc;
}
