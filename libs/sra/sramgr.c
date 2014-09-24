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

#include <sra/extern.h>
#include <sra/sradb.h>
#include <vdb/manager.h>
#include <vdb/schema.h>
#include <klib/refcount.h>
#include <klib/log.h>

#include <klib/out.h>

#include <klib/rc.h>
#include <sysalloc.h>

#include "libsradb.vers.h"
#include "sra-priv.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>


/* MakeRead
 *  create library reference for read-only access
 *
 *  "wd" [ IN, NULL OKAY ] - optional working directory for
 *  accessing the file system. mgr will attach its own reference
 *
 *  NB - not implemented in update libary
 *  and the read-only library may not be mixed with read/write
 */
LIB_EXPORT rc_t CC SRAMgrMakeRead ( const SRAMgr **mgr )
{
    return SRAMgrMakeReadWithDir ( mgr, NULL );
}

LIB_EXPORT rc_t CC SRAMgrMakeReadWithDir ( const SRAMgr **mgrp,
        struct KDirectory const *wd )
{
    rc_t rc;

    if ( mgrp == NULL )
        rc = RC ( rcSRA, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        const VDBManager *vmgr;
        rc = VDBManagerMakeRead ( & vmgr, wd );
        if ( rc == 0 )
        {
            SRAMgr *mgr;
            rc = SRAMgrMake ( & mgr, vmgr, wd );
            if ( rc == 0 )
            {
                * mgrp = mgr;
                return 0;
            }

            VDBManagerRelease ( vmgr );
        }

        * mgrp = NULL;
    }

    return rc;
}


/* GetSchema
 *  get a reference to the schema used to create new tables
 *
 *  "schema" [ OUT ] - return parameter for reference to SRA schema
 *
 * NB - returns a new reference that must be released via VSchemaRelease
 */
LIB_EXPORT rc_t CC SRAMgrGetSchemaRead ( const SRAMgr *self,
        const VSchema **schema )
{
    rc_t rc;

    if ( schema == NULL )
        rc = RC ( rcSRA, rcSchema, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcSRA, rcMgr, rcAccessing, rcSelf, rcNull );
        else
        {
            rc = VSchemaAddRef ( self -> schema );
            if ( rc == 0 )
            {
                * schema = self -> schema;
                return 0;
            }
        }

        * schema = NULL;
    }

    return rc;
}

/* OpenDatatypes
 *  open datatype registry object for requested access
 *
 *  "dt" [ OUT ] - return parameter for datatypes object
 */
LIB_EXPORT rc_t CC SRAMgrOpenDatatypesRead ( const SRAMgr *self,
        struct VDatatypes const **dt )
{
    return SRAMgrGetSchemaRead ( self, ( const VSchema** ) dt );
}


/* UseSchema
 *  allows SRA schema to be entirely replaced
 *  affects creation/update of all subsequent tables
 *
 *  "schema" [ IN ] - new schema to be applied
 *
 * NB - manager will attach a new reference to schema object,
 *  you are still responsible for releasing it via VSchemaRelease
 */
LIB_EXPORT rc_t CC SRAMgrUseSchemaRead ( const SRAMgr *self,
        const VSchema *schema )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcSRA, rcMgr, rcUpdating, rcSelf, rcNull );
    else if ( schema == NULL )
        rc = RC ( rcSRA, rcMgr, rcUpdating, rcSchema, rcNull );
    else if ( schema == self -> schema )
        rc = 0;
    else
    {
        rc = VSchemaAddRef ( schema );
        if ( rc == 0 )
        {
            VSchemaRelease ( self -> schema );
            ( ( SRAMgr* ) self ) -> schema = schema;
        }
    }

    return rc;
}


/* Version
 *  returns the library version
 */
LIB_EXPORT rc_t CC SRAMgrVersion ( const SRAMgr *self,
        uint32_t *version )
{
    if ( version == NULL )
        return RC ( rcSRA, rcMgr, rcAccessing, rcParam, rcNull );
    if ( self == NULL )
        return RC ( rcSRA, rcMgr, rcAccessing, rcSelf, rcNull );

    * version = LIBSRADB_VERS;
    return 0;
}
