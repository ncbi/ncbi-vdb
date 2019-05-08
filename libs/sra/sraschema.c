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

#include <sra/sch-extern.h>
#include <sra/sradb-priv.h>
#include <sra/sraschema.h>
#include <vdb/manager.h>
#include <vdb/schema.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include "sraschema-priv.h"
#include "sra-priv.h"


/*--------------------------------------------------------------------------
 * SRASchema
 *  a schema object pre-loaded with default SRA schema
 */

/* Make - DEPRECATED
 *  create an instance of the default SRA schema
 */
LIB_EXPORT rc_t CC SRASchemaMake ( VSchema **schema, const VDBManager *mgr )
{
    rc_t rc;

    if ( schema == NULL )
        rc = RC ( rcVDB, rcMgr, rcCreating, rcParam, rcNull );
    else
    {
        if ( mgr == NULL )
            rc = RC ( rcVDB, rcMgr, rcCreating, rcSelf, rcNull );
        else
        {
            rc = VDBManagerMakeSchema ( mgr, schema );
            if ( rc == 0 )
            {
/* SRASchemaMake is deprecated: when it is called - return an empty schema 
                rc = VSchemaParseText ( * schema, "sra-schema",
                                        sra_schema_text, sra_schema_size ); */
                if ( rc == 0 )
                    return 0;

                VSchemaRelease ( * schema );
            }
        }

        * schema = NULL;
    }
    return rc;
}
