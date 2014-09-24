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

#include <simple/extern.h>
#include "sra-exception.h"
#include "sra-string.h"
#include "vdb-mgr.h"
#include "vdb-database.h"
#include "vdb-table.h"

#include <vdb/manager.h>
#include <vdb/database.h>
#include <vdb/table.h>
#include <atomic.h>

#include <assert.h>


/*--------------------------------------------------------------------------
 * VDBMgr
 *  basically a front to VDBManager, but
 *  may also include certain caches for supporting Java
 */


/* MakeTable
 *  open object as a table
 */
struct SRA_Object *VDBMgrMakeTable ( const VDBMgr *self,
    struct SRAException *x, struct SRAString const *spec )
{
    const VTable *tbl;
    rc_t rc = VDBManagerOpenTableRead ( self, & tbl, NULL,
        "%.*s", ( int ) spec -> size, spec -> addr );
    if ( rc == 0 )
    {
        struct SRA_Object *obj = VDBTableObjMake ( x, tbl );
        VTableRelease ( tbl );
        return obj;
    }

    SRAExceptionError ( x, 0, rc, "failed to open table '%.*s'", ( uint32_t ) spec -> size, spec -> addr );
    return NULL;
}

/* MakeDatabase
 *  open object as a database
 */
struct SRA_Object *VDBMgrMakeDatabase ( const VDBMgr *self,
    struct SRAException *x, struct SRAString const *spec )
{
    const VDatabase *db;
    rc_t rc = VDBManagerOpenDBRead ( self, & db, NULL,
        "%.*s", ( int ) spec -> size, spec -> addr );
    if ( rc == 0 )
    {
        struct SRA_Object *obj = VDBDatabaseObjMake ( x, db );
        VDatabaseRelease ( db );
        return obj;
    }

    SRAExceptionError ( x, 0, rc, "failed to open database '%.*s'", ( uint32_t ) spec -> size, spec -> addr );
    return NULL;
}
