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
#include <sra/wsradb.h>
#include <sra/types.h>
#include <vdb/schema.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>
#include <kdb/meta.h>
#include <klib/printf.h> /* string_vprintf */
#include <klib/refcount.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "sra-debug.h"

#define KONST
#include "sra-priv.h"

#define RC_MODULE (rcSRA)
#define RC_TARGET (rcTable)
#define CLASS "SRATable"

/* Whack
 */

rc_t SRATableWhack ( SRATable *self )
{
    SRATableDestroy( self );
    return 0;
}

/* Create
 *  creates a new table
 *
 *  "tbl" [ OUT ] - return parameter for table
 *
 *  "path" [ IN ] - NUL terminated table name
 */
LIB_EXPORT rc_t CC SRAMgrCreateTable(SRAMgr *self, SRATable **tbl, const char *typespec, const char *path, ...) {
    va_list va;
    rc_t rc;
    
    va_start(va, path);
    rc = SRAMgrVCreateTable(self, tbl, typespec, path, va);
    va_end(va);
    return rc;
}

LIB_EXPORT rc_t CC SRAMgrVCreateTable ( SRAMgr *self, SRATable **rslt,
    const char *typespec, const char *spec, va_list args )
{
    rc_t rc;

    if ( rslt == NULL )
        rc = RC ( rcSRA, rcTable, rcConstructing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcSRA, rcTable, rcConstructing, rcSelf, rcNull );
        else if ( spec == NULL )
            rc = RC ( rcSRA, rcTable, rcConstructing, rcString, rcNull );
        else if ( spec [ 0 ] == 0 )
            rc = RC ( rcSRA, rcTable, rcConstructing, rcString, rcEmpty );
        else
        {
            char path [ 4097 ];
            size_t act_size = 0;
            
            rc = string_vprintf(path, 4097, &act_size, spec, args);
            path[act_size] = '\0';
            if ( rc == 0 )
            {
                VTable *vtbl;
                rc = VDBManagerCreateTable ( self -> vmgr, & vtbl, self -> schema,
                                             typespec, ( self -> mode & kcmBitMask ) | kcmCreate, "%s", path );
                if ( rc == 0 )
                {
                    rc = VTableColumnCreateParams ( vtbl, kcmCreate, kcsCRC32, 0 );
                    if ( rc == 0 )
                    {
                        SRATable *tbl = calloc ( 1, sizeof * tbl );
                        if ( tbl == NULL )
                            rc = RC ( rcSRA, rcTable, rcConstructing, rcMemory, rcExhausted );
                        else
                        {
                            tbl -> vtbl = vtbl;

                            rc = VTableOpenMetadataUpdate ( vtbl, & tbl -> meta );
                            if ( rc == 0 )
                                rc = KMetadataVersion ( tbl -> meta, & tbl -> metavers );
                            if ( rc == 0 )
                                rc = VTableCreateCursorWrite ( vtbl, & tbl -> curs, kcmInsert );
                            if ( rc == 0 )
                            {
                                tbl -> mgr = SRAMgrAttach ( self );
                                tbl -> mode = self -> mode;
                                tbl -> read_only = false;
                                KRefcountInit ( & tbl -> refcount, 1, "SRATable", "OpenTableUpdate", path );
                                VectorInit ( & tbl -> wcol, 0, 16 );
                                * rslt = tbl;
                                return 0;
                            }

                            vtbl = NULL;
                            SRATableWhack ( tbl );
                        }
                    }

                    VTableRelease ( vtbl );
                }
            }
        }

        * rslt = NULL;
    }

    return rc;
}


/* OpenUpdate
 *  open an existing table
 *
 *  "run" [ OUT ] - return parameter for table
 *
 *  "path" [ IN ] - NUL terminated table name
 */
LIB_EXPORT rc_t CC SRAMgrOpenTableUpdate(SRAMgr *self, SRATable **tbl, const char *path, ...) {
    va_list va;
    rc_t rc;
    
    va_start(va, path);
    rc = SRAMgrVOpenTableUpdate(self, tbl, path, va);
    va_end(va);
    return rc;
}

LIB_EXPORT rc_t CC SRAMgrVOpenTableUpdate(SRAMgr *mgr, SRATable **rslt, const char *spec, va_list args) {
    SRATable *self;
    char path[4096];
    rc_t rc;
    
    if (mgr == NULL)
        return RC(RC_MODULE, RC_TARGET, rcConstructing, rcSelf, rcNull);
    if (spec == NULL || rslt == NULL)
        return RC(RC_MODULE, RC_TARGET, rcConstructing, rcParam, rcNull);
    
    *rslt = NULL;
    
    rc = ResolveTablePath(mgr, path, sizeof(path), spec, args);
    if (rc)
        return rc;
    
    self = calloc(1, sizeof(*self));
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcConstructing, rcMemory, rcExhausted);
    
    rc = VDBManagerOpenTableUpdate(mgr->vmgr, &self->vtbl, mgr->schema, "%s", path);
    if (rc == 0) {
        rc = VTableOpenMetadataUpdate(self->vtbl, &self->meta);
        if (rc == 0) {
            rc = KMetadataVersion(self->meta, &self->metavers);
            if (rc == 0) {
                rc = VTableCreateCursorWrite(self->vtbl, &self->curs, kcmInsert);
                if (rc == 0) {
                    self->mgr = SRAMgrAttach(mgr);
                    self->mode = mgr->mode;
                    self->read_only = false;
                    KRefcountInit(&self->refcount, 1, "SRATable", "OpenTableUpdate", path);
                    
                    rc = SRATableFillOut ( self, true );
                    if ( rc == 0 )
                    {
                        VectorInit ( & self -> wcol, 0, 16 );
                    
                        *rslt = self;
                        return 0;
                    }
                }
            }
        }
    }
    SRATableWhack(self);
    return rc;
}

/* NewSpot
 *  creates a new spot record,
 *  returning spot id.
 *
 *  "id" [ OUT ] - return parameter for id of newly created spot
 */
LIB_EXPORT rc_t CC SRATableNewSpot( SRATable *self, spotid_t *id ) {
    rc_t rc;
    
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcInserting, rcSelf, rcNull);
    if (id == NULL)
        return RC(RC_MODULE, RC_TARGET, rcInserting, rcParam, rcNull);
    
    if (self->curs_open == false) {
        SRADBG(("opening cursor\n"));
        rc = VCursorOpen(self->curs);
        if (rc)
            return rc;
        self->curs_open = true;
    }
    
    rc = VCursorOpenRow(self->curs);
    if (rc == 0) {
        int64_t rowid;
        if( (rc = VCursorRowId(self->curs, &rowid)) == 0 ) {
            *id = rowid;
        }
    }
    return rc;
}

/* OpenSpot
 *  opens an existing spot record from id
 *
 *  "id" [ IN ] - 1-based spot id
 */
LIB_EXPORT rc_t CC SRATableOpenSpot( SRATable *self, spotid_t id ) {
    rc_t rc;
    
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcOpening, rcSelf, rcNull);
#if 0
    /* TODO: translate spot id to row id */
    rc = VCursorSetRowId(self->curs, id);
    if (rc == 0) {
        rc = VCursorOpenRow(self->curs);
    }
#else
    rc = RC(RC_MODULE, RC_TARGET, rcOpening, rcFunction, rcUnsupported);
#endif
    return rc;
}


/* CloseSpot
 *  closes a spot opened with either NewSpot or OpenSpot
 */
LIB_EXPORT rc_t CC SRATableCloseSpot( SRATable *self ) {
    rc_t rc;
    
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcClosing, rcSelf, rcNull);
    
    rc = VCursorCommitRow(self->curs);
    if (rc == 0)
        return VCursorCloseRow(self->curs);
    VCursorCloseRow(self->curs);
    return rc;
}


/* Commit
 *  commit all changes
 */
LIB_EXPORT rc_t CC SRATableCommit( SRATable *self ) {
    rc_t rc;
    
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcCommitting, rcSelf, rcNull);
    rc = VCursorCommit(self->curs);
    VCursorRelease(self->curs);
    self->curs = NULL;
    if (rc == 0)
        rc = VTableReindex(self->vtbl);
    return rc;
}

static int64_t CC cmp_index( const void *A, const void *B ) {
    return (int64_t)(*(const uint32_t *)A) - (int64_t)((const SRAColumn *)B)->idx;
}

static bool find_by_index(const Vector *vec, uint32_t idx, uint32_t *cndx) {
    uint32_t found;
    
    if ( VectorFind(vec, &idx, &found, cmp_index ) != NULL ) {
        *cndx = found;
        return true;
    }
    return false;
}

/* OpenColumnWrite
 *  open a column for write
 *
 *  "idx" [ OUT ] - return parameter for 1-based column index.
 *
 *  "col" [ OUT, NULL OKAY ] - optional return parameter for
 *  newly opened column.
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving column name
 *
 *  "datatype" [ IN ] - NUL terminated string in ASCII
 *   describing fully qualified column data type
 */
LIB_EXPORT rc_t CC SRATableOpenColumnWrite ( SRATable *self,
    uint32_t *idx, SRAColumn **col, const char *name, const char *datatype )
{
    rc_t rc;
    SRAColumn *rslt;
    uint32_t ndx, cndx;
    
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcOpening, rcSelf, rcNull);
    
    if (name == NULL || idx == NULL)
        return RC(RC_MODULE, RC_TARGET, rcOpening, rcParam, rcNull);
    
    *idx = 0;
    
    if (datatype && datatype[0])
    {
        SRADBG(("adding column (%s)%s\n", datatype, name));
        rc = VCursorAddColumn(self->curs, &ndx, "(%s)%s", datatype, name);
    }
    else
    {
        SRADBG(("adding column %s\n", name));
        rc = VCursorAddColumn(self->curs, &ndx, "%s", name);
    }
    
    if (rc != 0)
    {
        /* it's okay if the column is already there
           any other rc is a hard error */
        if (GetRCState ( rc ) != rcExists)
            return rc;
        
        if ( ! find_by_index(&self->wcol, ndx, &cndx) )
        {
            /* severe internal error */
            return RC ( RC_MODULE, RC_TARGET, rcOpening, rcNoObj, rcNotFound );
        }

        /* get the uncounted reference to column from table */
        rslt = VectorGet(&self->wcol, cndx);
    }
    else
    {
        VTypedecl type;
        VTypedesc desc;
        
        rslt = malloc(sizeof *rslt );
        if (rslt == NULL)
            return RC(RC_MODULE, rcColumn, rcConstructing, rcMemory, rcExhausted);
        
        rc = VCursorDatatype(self->curs, ndx, &type, &desc);
        if (rc == 0)
        {
            /* this object will sit uncounted within the table
               when the table goes away, it will douse the columns
               without regard to their reference count. see below */
            KRefcountInit(&rslt->refcount, 0, "SRAColumn", "OpenColumnWrite", name);

            /* the column has no reference to the table, so after this
               there will only be an uncounted reference from table to column */
            rslt->tbl = NULL;
            rslt->idx = ndx;
            rslt->read_only = false;
            rslt->elem_bits = VTypedescSizeof(&desc);

            rc = VectorAppend(&self->wcol, &cndx, rslt);
        }

        if ( rc != 0 )
        {
            free ( rslt );
            return rc;
        }
    }

    /* see if user wants a reference */
    if ( col != NULL )
    {
        /* the first column reference exported will take
           the refcount from zero to one */
        SRAColumnAddRef ( rslt );

        /* the first exported reference will need to be reflected
           to the table. this will ensure that the table never tries
           to whack its columns as long as they are held externally,
           because the table itself will be kept alive. when the last
           column reference is released, it will also release the table */
        if ( rslt -> tbl == NULL )
            rslt -> tbl = SRATableAttach ( self );

        *col = rslt;
    }

    *idx = cndx + 1;
    return rc;
}

static
rc_t lookup_and_validate(SRATable *self, const SRAColumn **rslt, uint32_t idx, const void *base, bitsz_t offset, bitsz_t size) {
    const SRAColumn *col;
    
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcWriting, rcSelf, rcNull);
    
    col = VectorGet(&self->wcol, idx - 1);
    if (col == NULL)
        return RC(RC_MODULE, RC_TARGET, rcWriting, rcParam, rcInvalid);
    
    if (size % col->elem_bits != 0)
        return RC(RC_MODULE, RC_TARGET, rcWriting, rcParam, rcInvalid);
    
    if (offset % col->elem_bits != 0)
        return RC(RC_MODULE, RC_TARGET, rcWriting, rcParam, rcInvalid);
    
    *rslt = col;
    return 0;
}

/* SetIdxColumnDefault
 *  give a default value for column
 *
 *  if no value gets written to a column within an open spot,
 *  this value is substituted.
 *
 *  "idx" [ IN ] - 1-based column index
 *
 *  "base" [ IN ] and "offset" [ IN ] - pointer and bit offset
 *  to start of row data
 *
 *  "size" [ IN ] - size in bits of row data
 */
LIB_EXPORT rc_t CC SRATableSetIdxColumnDefault ( SRATable *self,
        uint32_t idx, const void *base, bitsz_t offset, bitsz_t size ) {
    const SRAColumn *col;
    rc_t rc;
    
    rc = lookup_and_validate(self, &col, idx, base, offset, size);
    if (rc == 0)
        rc = VCursorDefault(self->curs, col->idx, col->elem_bits, base, offset / col->elem_bits, size / col->elem_bits);
    return rc;
}


/* WriteIdxColumn
 *  write row data to an indexed column
 *
 *  "idx" [ IN ] - 1-based column index
 *
 *  "base" [ IN ] and "offset" [ IN ] - pointer and bit offset
 *  to start of row data
 *
 *  "size" [ IN ] - size in bits of row data
 */
LIB_EXPORT rc_t CC SRATableWriteIdxColumn ( SRATable *self,
        uint32_t idx, const void *base, bitsz_t offset, bitsz_t size ) {
    const SRAColumn *col;
    rc_t rc;
    
    rc = lookup_and_validate(self, &col, idx, base, offset, size);
    if (rc == 0)
        rc = VCursorWrite(self->curs, col->idx, col->elem_bits, base, offset / col->elem_bits, size / col->elem_bits);
    return rc;
}


/* MetaFreeze
 *  freezes current metadata revision
 *  further modification will begin on a copy
 */
LIB_EXPORT rc_t CC SRATableMetaFreeze ( SRATable *self ) {
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcOpening, rcSelf, rcNull);
    
    return KMetadataFreeze(self->meta);
}


/* OpenMDataNode
 *  open a metadata node
 */
LIB_EXPORT rc_t CC SRATableOpenMDataNodeUpdate ( SRATable *self, struct KMDataNode **node, const char *path, ... ) {
    va_list va;
    rc_t rc;
    
    va_start(va, path);
    rc = SRATableVOpenMDataNodeUpdate(self, node, path, va);
    va_end(va);
    return rc;
}

LIB_EXPORT rc_t CC SRATableVOpenMDataNodeUpdate ( SRATable *self, struct KMDataNode **node, const char *spec, va_list args ) {
    rc_t rc;
    char path[4096];
    int n;
    
    if (self == NULL)
        return RC(RC_MODULE, RC_TARGET, rcOpening, rcSelf, rcNull);
    if (spec == NULL)
        return RC(RC_MODULE, RC_TARGET, rcOpening, rcParam, rcNull);
    
    n = vsnprintf(path, sizeof(path), spec, args);
    if (n >= sizeof(path))
        return RC(RC_MODULE, RC_TARGET, rcOpening, rcName, rcTooLong);
    
    rc = KMetadataOpenNodeUpdate(self->meta, node, "%s", path);
    if (rc)
    {
        SRADBG(("failed to open table metadata node '%s' %R", path, rc));
    }
    return rc;
}

#define RD_COLVALUE(tbl, nm, var) \
                SRATable_ReadColBack(tbl, nm, sizeof(var) * 8, (void*)&var, sizeof(var))

static
rc_t SRATable_ReadColBack(const VTable* tbl, const char* col_name, uint32_t elem_bits, void *buffer, uint32_t blen)
{
    rc_t rc = 0;
    uint32_t idx;
    const VCursor* curs = NULL;
   
    if( (rc = VTableCreateCursorRead(tbl, &curs)) == 0 &&
        (rc = VCursorAddColumn(curs, &idx, "%s", col_name)) == 0 &&
        (rc = VCursorOpen(curs)) == 0 ) {
        uint32_t len;
        rc = VCursorReadDirect(curs, 1, idx, elem_bits, buffer, blen, &len);
    }
    VCursorRelease(curs);
    return rc;
}

LIB_EXPORT rc_t CC SRATableBaseCount ( const SRATable *self, uint64_t *rslt )
{
    rc_t rc;

    if ( rslt == NULL )
        rc = RC ( rcSRA, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcSRA, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            RD_COLVALUE(self->vtbl, "BASE_COUNT", self->base_count);
            *rslt = self->base_count;
            return 0;
        }
        *rslt = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC SRATableSpotCount ( const SRATable *self, uint64_t *rslt )
{
    rc_t rc;

    if ( rslt == NULL )
        rc = RC ( rcSRA, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcSRA, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            RD_COLVALUE(self->vtbl, "SPOT_COUNT", self->spot_count);
            *rslt = self->spot_count;
            return 0;
        }
        *rslt = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC SRATableMinSpotId ( const SRATable *self, spotid_t *rslt )
{
    rc_t rc;

    if ( rslt == NULL )
        rc = RC ( rcSRA, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcSRA, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            RD_COLVALUE(self->vtbl, "MIN_SPOT_ID", self->min_spot_id);
            *rslt = self->min_spot_id;
            return 0;
        }
        *rslt = 0;
    }
    return rc;
}

LIB_EXPORT rc_t CC SRATableMaxSpotId ( const SRATable *self, spotid_t *rslt )
{
    rc_t rc;

    if ( rslt == NULL )
        rc = RC ( rcSRA, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcSRA, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            RD_COLVALUE(self->vtbl, "MAX_SPOT_ID", self->max_spot_id);
            *rslt = self->max_spot_id;
            return 0;
        }
        *rslt = 0;
    }
    return rc;
}
