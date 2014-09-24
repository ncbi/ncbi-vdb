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

#include "dbmgr-priv.h"
#include "table-priv.h"
#include "schema-priv.h"
#include "schema-parse.h"
#include "linker-priv.h"

#undef KONST
#undef SKONST

#include <vdb/table.h>
#include <vdb/vdb-priv.h>
#include <kdb/meta.h>
#include <kdb/namelist.h>
#include <kfs/dyload.h>
#include <klib/namelist.h>
#include <klib/token.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


/* LoadSchema
 *  looks in metadata for stored schema
 *  or searches available definitions for match
 */
typedef struct VTableFindData VTableFindData;
struct VTableFindData
{
    VLinker *linker;
    struct KDlset *libs;
    const VTable *vtbl;
    const STable *stbl;
};

static
bool CC VTableTestUntyped ( void *item, void *data )
{
    VTableFindData *pb = data;
    const STable *stbl = ( const void* ) item;

    if ( stbl -> untyped != NULL )
    {
        /* get the function pointer */
        VUntypedTableTest test = NULL;
        rc_t rc = VLinkerFindUntyped ( pb -> linker, pb -> libs,
            & test, stbl -> untyped );
        if ( rc == 0 )
        {
            /* ask if it recognizes the table */
            if ( ( * test ) ( pb -> vtbl -> ktbl, pb -> vtbl -> meta ) )
            {
                pb -> stbl = stbl;
                return true;
            }
        }
    }
     
    return false;
}

static
bool VSchemaFindUntyped ( const VSchema *self, VTableFindData *pb )
{
    if ( self -> dad )
    {
        if ( VSchemaFindUntyped ( self -> dad, pb ) )
            return true;
    }
    
    return VectorDoUntil ( & self -> tbl, false, VTableTestUntyped, pb );
}

static
rc_t VTableFindType ( VTable *self )
{
    rc_t rc;
    VTableFindData pb;
    
    pb . linker = self -> linker;
    
    rc = VLinkerOpen ( pb . linker, & pb . libs );
    if ( rc == 0 )
    {
        pb . vtbl = self;
        pb . stbl = NULL;
        
        /* search schema */
        VSchemaFindUntyped ( self -> schema, & pb );
        
        /* record stbl */
        self -> stbl = pb . stbl;
        KDlsetRelease ( pb . libs );
    }
    return rc;
}

static
rc_t VTableLoadSchemaNode ( VTable *self, const KMDataNode *node )
{
    rc_t rc;
    
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
        "VTableLoadSchema", KMDataNodeFillSchema, & pb );
    if ( rc == 0 )
    {
        /* determine table type */
        rc = KMDataNodeReadAttr ( node, "name",
            pb . buff, sizeof pb . buff, & pb . pos );
        if ( rc == 0 )
        {
            uint32_t type;
            const SNameOverload *name;
            
            /* find the stbl if possible */
            self -> stbl = VSchemaFind ( self -> schema,
                & name, & type, pb . buff, "VTableLoadSchema", false );
            
            /* the schema must be found in this case */
            if ( self -> stbl == NULL || type != eTable )
            {
                self -> stbl = NULL;
                rc = RC ( rcVDB, rcTable, rcLoading, rcSchema, rcCorrupt );
                PLOGERR ( klogInt, ( klogInt, rc, "failed to establish table type from '$(expr)'",
                                     "expr=%s", pb . buff ));
            }
        }
    }
    return rc;
}

rc_t VTableLoadSchema ( VTable *self )
{
    /* try to get schema text */
    const KMDataNode *node;
    rc_t rc = KMetadataOpenNodeRead ( self -> meta, & node, "schema" );
    if ( rc == 0 )
    {
        rc = VTableLoadSchemaNode ( self, node );
        KMDataNodeRelease ( node );
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        /* this is an older version. search schema
         for a table having an untyped function */
        rc = VTableFindType ( self );
    }

    return rc;
}

/* ExtendSchema
 *  discover all existing physical and static columns
 *  that are not represented in the table schema
 *  and add them in ( if possible )
 */
static
rc_t VTablePopulateStrayKColumn ( const VTable *self, const char *name )
{
    /* TBD - not returning error because the result of doing
       nothing is simply to leave these columns inaccessible  */

    /* open KColumn */
    /* open KMetadata */
    /* process v1 */
    /* process v2 */
    /* need to discover a datatype and schema information */
    return 0;
}

static
rc_t VTablePopulateStrayStatic ( const VTable *self,
    const char *name, const KMDataNode *node )
{
    /* TBD - not returning error because the result of doing
       nothing is simply to leave these columns inaccessible  */

    /* need to discover a datatype */
    return 0;
}

static
bool VTableNameAvail ( const KSymTable *tbl, const char *name )
{
    String str;

    /* build a physical name from simple name */
    char pname [ 256 ];
    int len = snprintf ( pname, sizeof pname, ".%s", name );
    if ( len < 0 || len >= sizeof pname )
        return false;

    /* test for defined physical name */
    StringInit ( & str, pname, len, len );
    if ( KSymTableFind ( tbl, & str ) != NULL )
        return false;

    /* test for defined simple name */
    StringSubstr ( & str, & str, 1, 0 );
    if ( KSymTableFind ( tbl, & str ) != NULL )
        return false;

    /* name is available */
    return true;
}

LIB_EXPORT rc_t CC VTableExtendSchema ( const VTable *self )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcVDB, rcTable, rcUpdating, rcSelf, rcNull );
    else
    {
        /* create symbol table for STable */
        KSymTable tbl;
        rc = init_tbl_symtab ( & tbl, self -> schema, self -> stbl );
        if ( rc == 0 )
        {
            KNamelist *names;
            const char *name;
            uint32_t i, count;

            /* list all physical tables from KTable */
            rc = KTableListCol ( self -> ktbl, & names );
            if ( rc != 0 )
                rc = 0;
            else
            {
                rc = KNamelistCount ( names, & count );
                for ( i = 0; rc == 0 && i < count; ++ i )
                {
                    /* if there are any stray columns, add them in */
                    rc = KNamelistGet ( names, i, & name );
                    if ( rc == 0 && VTableNameAvail ( & tbl, name ) )
                        rc = VTablePopulateStrayKColumn ( self, name );
                }

                KNamelistRelease ( names );
            }

            if ( rc == 0 )
            {
                /* access table's static columns */
                const KMDataNode *cols = self -> col_node;
                if ( cols != NULL )
                {
                    /* list all columns */
                    rc = KMDataNodeListChild ( cols, & names );
                    if (rc == 0) {
                        rc = KNamelistCount ( names, & count );
                        for ( i = 0; rc == 0 && i < count; ++ i )
                        {
                            rc = KNamelistGet ( names, i, & name );
                            if ( rc == 0 && VTableNameAvail ( & tbl, name ) )
                            {
                                const KMDataNode *node;
                                rc = KMDataNodeOpenNodeRead ( cols, & node, "%s", name );
                                if ( rc == 0 )
                                {
                                    /* add in static column */
                                    rc = VTablePopulateStrayStatic ( self, name, node );
                                    KMDataNodeRelease ( node );
                                }
                            }
                        }
                        KNamelistRelease ( names );
                    }
                }
            }
        }

        KSymTableWhack ( & tbl );
    }

    return rc;
}
