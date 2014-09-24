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

#define KONST const
#define SKONST

#include "database-priv.h"
#include "dbmgr-priv.h"
#include "schema-parse.h"

#undef KONST
#undef SKONST

#include "schema-priv.h"
#include "linker-priv.h"

#include <vdb/manager.h>
#include <vdb/database.h>
#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/meta.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VDatabase
 *  opaque connection to a database within file system
 */


/* LoadSchema
 */

rc_t VDatabaseLoadSchema ( VDatabase *self )
{
    /* try to get schema text */
    const KMDataNode *node;
    rc_t rc = KMetadataOpenNodeRead ( self -> meta, & node, "schema" );
    if ( rc == 0 )
    {
        /* the node is probably within our 4K buffer,
           but by using the callback mechanism we don't
           have buffer or allocation issues. */
        KMDataNodeSchemaFillData pb;
        pb . node = node;
        pb . pos = 0;
        pb . add_v0 = false;

        /* add in schema text. it is not mandatory, but it is
           the design of the system to store object schema with
           the object so that it is capable of standing alone */
        rc = VSchemaParseTextCallback ( self -> schema,
            "VDatabaseLoadSchema", KMDataNodeFillSchema, & pb );
        if ( rc == 0 )
        {
            /* determine database type */
            rc = KMDataNodeReadAttr ( node, "name",
                pb . buff, sizeof pb . buff, & pb . pos );
            if ( rc == 0 )
            {
                uint32_t type;
                const SNameOverload *name;

                /* find the sdb if possible */
                self -> sdb = VSchemaFind ( self -> schema,
                    & name, & type, pb . buff, "VDatabaseLoadSchema", false );

                /* the schema must be found in this case */
                if ( self -> sdb == NULL || type != eDatabase )
                {
                    self -> sdb = NULL;
                    rc = RC ( rcVDB, rcDatabase, rcLoading, rcSchema, rcCorrupt );
                    PLOGERR ( klogInt, ( klogInt, rc, "failed to establish database type from '$(expr)'",
                                         "expr=%s", pb . buff ));
                }
            }
        }

        KMDataNodeRelease ( node );
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        /* the database may be under creation */
        if ( ! self -> read_only )
            rc = 0;
    }

    return rc;
}
