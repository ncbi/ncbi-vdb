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
#include <sra/srapath.h>
#include <sra/types.h>
#include <sra/sraschema.h>
#include <sra/sradb-priv.h>
#include <vdb/database.h>
#include <vdb/schema.h>
#include <vdb/table.h>
#include <vdb/cursor.h>
#include <vdb/vdb-priv.h>
#include <vdb/dependencies.h>
#include <kdb/meta.h>
#include <kdb/manager.h>
#include <kdb/table.h>
#include <kdb/database.h>
#include <kdb/kdb-priv.h>
#include <vfs/manager.h>
#include <vfs/path.h>
#include <vfs/path-priv.h>
#include <vfs/resolver.h>
#include <klib/refcount.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <klib/text.h>
#include <klib/printf.h>
#include <kfs/toc.h>
#include <kfs/file.h>
#include <sysalloc.h>

#include "sra-priv.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <va_copy.h>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

/* Destroy
 */
static
void CC column_release ( void *item, void *ignore )
{
    SRAColumn *self = item;
    self -> tbl = NULL;
    SRAColumnWhack ( self );
}

void SRATableDestroy ( SRATable *self )
{
    VectorWhack ( & self -> wcol, column_release, NULL );
    VCursorRelease(self->curs);
    KMetadataRelease ( self -> meta );
    VTableRelease ( self -> vtbl );
    SRAMgrSever ( self -> mgr );

    memset(self, 0, sizeof *self);

    free ( self );
}

/* AddRef
 * Release
 *  see REFERENCE COUNTING, above
 */
LIB_EXPORT rc_t CC SRATableAddRef( const SRATable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "SRATable" ) )
        {
        case krefLimit:
            return RC ( rcSRA, rcTable, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC SRATableRelease( const SRATable *self )
{
    rc_t rc = 0;
    
    if (self)
    {
        switch (KRefcountDrop(&self->refcount, "SRATable"))
        {
        case krefWhack:
            return SRATableWhack ( ( SRATable* ) self );
        case krefNegative:
            rc = RC (rcSRA, rcTable, rcDestroying, rcSelf, rcDestroyed);
            PLOGERR (klogInt,(klogInt, rc, "Released an SRATable $(B) with no more references",
                      PLOG_P(self)));
            break;
        }
    }
    return rc;
}

/* Attach
 * Sever
 */
SRATable *SRATableAttach ( const SRATable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAddDep ( & self -> refcount, "SRATable" ) )
        {
        case krefLimit:
            return NULL;
        }
    }
    return ( SRATable* ) self;
}

rc_t SRATableSever ( const SRATable *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDropDep ( & self -> refcount, "SRATable" ) )
        {
        case krefWhack:
            return SRATableWhack ( ( SRATable* ) self );
        case krefNegative:
            return RC ( rcSRA, rcTable, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* FillOutTableRead
 *  creates an empty cursor
 *  accesses metadata
 */
static rc_t ReadSpotSequence_v1(SRATable *self)
{
    const KMDataNode *n;
    rc_t rc = KMetadataOpenNodeRead(self->meta, &n, ".seq");
    if (rc == 0)
    {
        rc = KMDataNodeReadAsU64(n, &self->spot_count);
        KMDataNodeRelease(n);
    }
    return rc;
}

typedef enum {
    eNotRead,
    eNotFound,
    eFailed,
    eRead
} EState;
typedef struct {
    EState state;
    rc_t rc;
} State;
typedef struct {
    uint64_t value;
    State state;
} U64;
typedef struct {
    uint32_t value;
    State state;
} U32;
typedef struct {
    U64 BASE_COUNT;
    U32 MAX_SPOT_ID;
    U32 MIN_SPOT_ID;
    U64 SPOT_COUNT;
} PseudoMeta;
static
rc_t VCursor_ReadPseudoMeta(rc_t rc, const VCursor *self,
    const char *name, void *buffer, uint32_t blen, State *state)
{
    uint32_t idx = ~0;
    uint32_t row_len = ~0;

    assert(state);
    state->rc = 0;

    if (rc != 0)
    {   return rc; }

    state->state = eNotRead;

    rc = VCursorAddColumn(self, &idx, "%s", name);

    if (rc != 0) {
        state->rc = rc;

        if ( GetRCObject( rc ) == (enum RCObject)rcColumn && GetRCState( rc ) == rcNotFound ) {
            rc = 0;
            state->state = eNotFound;
        }
        else {
            state->state = eFailed;
        }
    }

    if (state->rc == 0) {
        rc = VCursorReadDirect(self, 1, idx, blen * 8, buffer, blen, &row_len);

        state->rc = rc;

        if (rc != 0)
        {   state->state = eFailed; }
        else
        {   state->state = eRead; }

    }

    return rc;
}

static
rc_t VCursor_ReadPseudoMetaU32(rc_t rc, const VCursor *self,
    const char *name, U32 *val)
{
    assert(val);
    return VCursor_ReadPseudoMeta(rc, self,
        name, &val->value, sizeof val->value, &val->state);
}

static
rc_t VCursor_ReadPseudoMetaU64(rc_t rc, const VCursor *self,
    const char *name, U64 *val)
{
    assert(val);
    return VCursor_ReadPseudoMeta(rc, self,
        name, &val->value, sizeof val->value, &val->state);
}

static
rc_t PseudoMetaInit(PseudoMeta *self, const VCursor *curs,
    bool readSpotCount, uint64_t spot_count)
{
    rc_t rc = 0;

    assert(self);

    memset(self, 0, sizeof *self);

    if (readSpotCount) {
        rc =
         VCursor_ReadPseudoMetaU64(rc, curs, "SPOT_COUNT", &self->SPOT_COUNT);
    }
    else {
        self->SPOT_COUNT.value = spot_count;
        self->SPOT_COUNT.state.state = eRead;
    }

    rc = VCursor_ReadPseudoMetaU64(rc, curs, "BASE_COUNT", &self->BASE_COUNT);

    rc = VCursor_ReadPseudoMetaU32(rc, curs, "MIN_SPOT_ID", &self->MIN_SPOT_ID); 

    rc = VCursor_ReadPseudoMetaU32(rc, curs, "MAX_SPOT_ID", &self->MAX_SPOT_ID);

    return rc;
}

static
rc_t PseudoMetaFix(PseudoMeta *self)
{
    rc_t rc = 0;

    assert(self);

    if (self->MIN_SPOT_ID.state.state != eRead)
    {   self->MIN_SPOT_ID.value = 1; }

    if (self->SPOT_COUNT.state.state != eRead &&
        self->MAX_SPOT_ID.state.state != eRead)
    {
        return self->SPOT_COUNT.state.rc;
    }
    else if (self->SPOT_COUNT.state.state == eRead) {
        if (self->MAX_SPOT_ID.state.state != eRead) {
            self->MAX_SPOT_ID.value
                = self->MIN_SPOT_ID.value + self->SPOT_COUNT.value - 1;
        }
        else if (self->MAX_SPOT_ID.value >= self->MIN_SPOT_ID.value) {
	    uint32_t delta = (uint32_t) (self->SPOT_COUNT.value - (self->MAX_SPOT_ID.value - self->MIN_SPOT_ID.value + 1)); /** SPOT_COUNT is 64 bit, but M*_SPOT_ID is 32; anticipate rollover **/
            if ( delta )
            {
                self->SPOT_COUNT.value
                    = self->MAX_SPOT_ID.value - self->MIN_SPOT_ID.value + 1;
            }
        }
    }
    else {
            self->SPOT_COUNT.value
                = self->MAX_SPOT_ID.value - self->MIN_SPOT_ID.value - 1;
    }

    return rc;
}

static
rc_t SRATableLoadMetadata(SRATable * self)
{
    rc_t rc = 0;
    PseudoMeta meta;
    bool readSpotCount = true;

    assert(self && self->curs != NULL);
    assert(self->curs_open == true);

    readSpotCount = self->metavers > 1;
    if (!readSpotCount) {
        rc = ReadSpotSequence_v1(self);
        if (rc != 0)
        {   return rc; }
    }

    rc = PseudoMetaInit(&meta, self->curs, readSpotCount, self->spot_count);
    if (rc != 0)
    {   return rc; }

    rc = PseudoMetaFix(&meta);
    if (rc != 0)
    {   return rc; }

    self->spot_count = meta.SPOT_COUNT.value;
    self->base_count = meta.BASE_COUNT.value;
    self->min_spot_id = meta.MIN_SPOT_ID.value;
    self->max_spot_id = meta.MAX_SPOT_ID.value;

    {
	int64_t delta = self->spot_count - (self->max_spot_id-self->min_spot_id+1);
	if(delta>0 && ((uint32_t)delta)==0){/* there was a rollover*/
		self->max_spot_id = self->min_spot_id + self->spot_count - 1;
	}
    }

    return rc;
}

/* detect min and max spot-id from a temp. cursor */
static rc_t SRATableGetMinMax( SRATable * self )
{
    const VCursor *temp_cursor;
    rc_t rc;

    assert( self != NULL );
    assert( self->vtbl != NULL);
    rc = VTableCreateCursorRead( self->vtbl, &temp_cursor );
    if ( rc == 0 )
    {
        uint32_t idx;
        rc = VCursorAddColumn ( temp_cursor, &idx, "READ" );
        if ( rc == 0 )
        {
            rc = VCursorOpen( temp_cursor );
            if ( rc == 0 )
            {
                int64_t  first;
                uint64_t count;
                rc = VCursorIdRange( temp_cursor, 0, &first, &count );
                if ( rc == 0 )
                {
                    self->min_spot_id = first;
                    self->max_spot_id = first + count;
                    self->spot_count = count;
                }
            }
        }
        VCursorRelease( temp_cursor );
    }
    return rc;
}

rc_t SRATableFillOut ( SRATable *self, bool update )
{
    rc_t rc;
    
    /* require these operations to succeed */
    rc = VCursorPermitPostOpenAdd( self->curs );
    if ( rc != 0 )
        return rc;
    rc = VCursorOpen( self->curs );
    if ( rc != 0 )
        return rc;
    self -> curs_open = true;
    if ( ! update )
    {
        rc = SRATableLoadMetadata( self );
        if ( rc != 0 )
            rc = SRATableGetMinMax( self );
    }
    return rc;
}


/* ResolveTablePath
 *  takes either an accession or path
 *  substitutes any arguments
 *  resolves via SRAPath mgr if present
 */
rc_t ResolveTablePath ( const SRAMgr *mgr,
    char *path, size_t psize, const char *spec, va_list args )
{
#if OLD_SRAPATH_MGR
    int len;
    char tblpath [ 4096 ];
    const SRAPath *pmgr = mgr -> _pmgr;

    /* if no path manager or if the spec string has embedded path separators,
       then this can't be an accession - just print it out */
    if ( mgr -> _pmgr == NULL || strchr( spec, '/' ) != NULL )
    {
        len = vsnprintf ( path, psize, spec, args );
        if ( len < 0 || ( size_t ) len >= psize )
            return RC ( rcSRA, rcTable, rcOpening, rcPath, rcExcessive );
        return 0;
    }

    /* create a copy - not likely to be too large */
    len = vsnprintf ( tblpath, sizeof tblpath, spec, args );
    if ( len < 0 || ( size_t ) len >= sizeof tblpath )
        return RC ( rcSRA, rcTable, rcOpening, rcPath, rcExcessive );

    /* test if the path exists in current directory, i.e. with assumed dot */
    if ( ! SRAPathTest ( pmgr, tblpath ) )
    {
        rc_t rc = SRAPathFind ( pmgr, tblpath, path, psize );
        if ( rc == 0 )
            return 0;
    }

    /* use the path given */
    if ( ( size_t ) len >= psize )
        return RC ( rcSRA, rcTable, rcOpening, rcBuffer, rcInsufficient );
    strcpy ( path, tblpath );

    return 0;
#else
    VFSManager *vfs;
    rc_t rc = VFSManagerMake ( & vfs );
    if ( rc == 0 )
    {
        VPath *accession;
        const VPath *tblpath = NULL;
        rc = VFSManagerVMakePath ( vfs, & accession, spec, args );
        if ( rc == 0 )
        {
            rc = VResolverLocal ( ( const VResolver* ) mgr -> _pmgr, accession, & tblpath );
            if ( rc == 0 )
            {
                size_t size;
                rc = VPathReadPath ( tblpath, path, psize, & size );
                VPathRelease ( tblpath );
            }
            VPathRelease ( accession );
        }

        VFSManagerRelease ( vfs );
    }
    return rc;
#endif
}

/* OpenRead
 *  open an existing table
 *
 *  "tbl" [ OUT ] - return parameter for table
 *
 *  "spec" [ IN ] - NUL terminated UTF-8 string giving path
 *  to table.
 */
static
rc_t CC SRAMgrVOpenAltTableRead ( const SRAMgr *self,
    const SRATable **rslt, const char *altname, const char *spec, va_list args )
{
    rc_t rc;

    if ( rslt == NULL )
        rc = RC ( rcSRA, rcTable, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcSRA, rcMgr, rcAccessing, rcSelf, rcNull );
        else if ( spec == NULL )
            rc = RC ( rcSRA, rcTable, rcOpening, rcName, rcNull );
        else if ( spec [ 0 ] == 0 )
            rc = RC ( rcSRA, rcTable, rcOpening, rcName, rcEmpty );
        else
        {
            SRATable *tbl = calloc ( 1, sizeof *tbl );
            if ( tbl == NULL )
                rc = RC ( rcSRA, rcTable, rcConstructing, rcMemory, rcExhausted );
            else
            {
                VSchema *schema = NULL;

                rc = VDBManagerMakeSRASchema(self -> vmgr, & schema);
                if ( rc == 0 ) 
                {
                    va_list args_copy;
                    va_copy ( args_copy, args );
                    rc = VDBManagerVOpenTableRead ( self -> vmgr, & tbl -> vtbl, schema, spec, args );
                    if ( rc != 0 && GetRCObject ( rc ) == (enum RCObject)rcTable && GetRCState ( rc ) == rcIncorrect )
                    {
                        const VDatabase *db;
                        rc_t rc2 = VDBManagerVOpenDBRead ( self -> vmgr, & db, schema, spec, args_copy );
                        if ( rc2 == 0 )
                        {
                            rc2 = VDatabaseOpenTableRead ( db, & tbl -> vtbl, "%s", altname );
                            if ( rc2 == 0 )
                                rc = 0;

                            VDatabaseRelease ( db );
                        }
                    }
                    va_end ( args_copy );

                    VSchemaRelease(schema);

                    if ( rc == 0 )
                    {
                        rc = VTableOpenMetadataRead ( tbl -> vtbl, & tbl -> meta );
                        if ( rc == 0 )
                        {
                            rc = KMetadataVersion ( tbl -> meta, & tbl -> metavers );
                            if ( rc == 0 )
                            {
                                rc = VTableCreateCursorRead ( tbl -> vtbl, & tbl -> curs );
                                if ( rc == 0 )
                                {
                                    tbl -> mode = self -> mode;
                                    tbl -> read_only = true;
                                    KRefcountInit ( & tbl -> refcount, 1, "SRATable", "OpenTableRead", spec );
                                        
                                    rc = SRATableFillOut ( tbl, false );
                                    if ( rc == 0 )
                                    {
                                        * rslt = tbl;
                                        return 0;
                                    }
                                }
                            }
                        }
                    }
                    
                }
                SRATableWhack ( tbl );
            }
        }

        * rslt = NULL;
    }
    return rc;
}

/* OpenRead
 *  open an existing table
 *
 *  "tbl" [ OUT ] - return parameter for table
 *
 *  "spec" [ IN ] - NUL terminated UTF-8 string giving path
 *  to table.
 */
LIB_EXPORT rc_t CC SRAMgrVOpenTableRead ( const SRAMgr *self,
        const SRATable **crslt, const char *spec, va_list args )
{
    rc_t rc;
    char tblpath [ 4096 ];
    int num_writ = vsnprintf ( tblpath, sizeof tblpath, spec, args );
    if ( num_writ < 0 || ( size_t ) num_writ >= sizeof tblpath )
        rc = RC ( rcSRA, rcMgr, rcOpening, rcPath, rcExcessive );
    else
    {
        SRATable **rslt = (SRATable **)crslt; /* to avoid "const_casts" below */
        rc = SRACacheGetTable( self->cache, tblpath, crslt );
        if  (rc == 0 )
        {
            if ( *crslt == NULL )
            {
                rc = SRAMgrOpenAltTableRead ( self, crslt, "SEQUENCE", tblpath );
                if ( rc == 0 )
                {
                    rc = SRACacheAddTable( self->cache, tblpath, *rslt);
                    if ( GetRCObject(rc) == rcParam && GetRCState(rc) == rcExists )
                    {           /* the same object has appeared in the cache since our call to SRACacheGetTable above;  */
                       rc = 0;  /* return the new object, never mind the cache */
                    }
                    else if ( ! SRACacheMetricsLessThan(&self->cache->current, &self->cache->hardThreshold) )
                       rc = SRACacheFlush(self->cache);
                }
            }
        }
        else if ( (GetRCObject(rc) == rcName && GetRCState(rc) == rcInvalid) ||   /* accessions with irregular names are not cached */
                  (GetRCObject(rc) == rcParam && GetRCState(rc) == rcBusy)    )   /* in cache but in use */
        {    
            rc = SRAMgrOpenAltTableRead ( self, crslt, "SEQUENCE", tblpath );
            if (rc == 0)
                (*rslt)->mgr = SRAMgrAttach(self);
        }
    }

    return rc;
}

LIB_EXPORT rc_t CC SRAMgrOpenTableRead ( const SRAMgr *self,
        const SRATable **tbl, const char *spec, ... )
{
    rc_t rc;
    
    va_list args;
    va_start ( args, spec );

    rc = SRAMgrVOpenTableRead ( self, tbl, spec, args );

    va_end ( args );

    return rc;
}


/* Read - PRIVATE
 *  column message sent via table
 */
rc_t SRATableRead ( const SRATable *self, spotid_t id, uint32_t idx,
    const void **base, bitsz_t *offset, bitsz_t *size )
{
    rc_t rc;

    if ( base == NULL || offset == NULL || size == NULL )
        rc = RC ( rcSRA, rcColumn, rcReading, rcParam, rcNull );
    else if ( self == NULL )
        rc = RC ( rcSRA, rcTable, rcAccessing, rcSelf, rcNull );
    else
    {
        rc = 0;

        /* open cursor */
        if ( ! self -> curs_open )
        {
            rc = VCursorOpen(self->curs);
            if ( rc == 0 )
                ((SRATable *)self)->curs_open = true;
        }

        if ( rc == 0 )
        {
            uint32_t elem_bits, elem_off, elem_cnt;
            rc = VCursorCellDataDirect ( self -> curs, id, idx,
                & elem_bits, base, & elem_off, & elem_cnt );
            if ( rc == 0 )
            {
                * offset = elem_off * elem_bits;
                * size   = elem_cnt * elem_bits;
                return 0;
            } else if( UIError(rc, NULL, self->vtbl) ) {
                UITableLOGError(rc, self->vtbl, true);
            }
        }
    }

    if ( base != NULL )
        * base = NULL;
    if ( offset != NULL )
        * offset = 0;
    if ( size != NULL )
        * size = 0;

    return rc;
}

/* GetSpotId
 *  convert spot name to spot id
 *
 *  "id" [ OUT ] - return parameter for min(spot_id)-max(spot_id)
 *
 *  "spot_name" [ IN ] - external spot name string
 *  in platform canonical format.
 */
LIB_EXPORT rc_t CC SRATableGetSpotId ( const SRATable *self,
        spotid_t *rslt, const char *spot_name )
{
    rc_t rc;
    if( self == NULL || spot_name == NULL){
        rc=RC(rcSRA, rcTable, rcListing, rcSelf, rcName);
    } else {
        rc=VCursorParamsSet((struct VCursorParams*)self->curs,"QUERY_BY_NAME" ,spot_name);
        if( rc == 0) {
            struct {
                uint64_t start_id;
                uint64_t id_count;
                int64_t x;
                int64_t y;
            } out;
            uint32_t idx,len;
            rc = VCursorAddColumn(self->curs, &idx, "SPOT_IDS_FOUND");
            if( rc == 0 || GetRCState(rc) == rcExists){
                rc = VCursorReadDirect(self->curs,1,idx,sizeof(out) * 8,&out, 1 , &len);
                if ( rc == 0 ) {
                    if(out.id_count==1) {
                        if(rslt) *rslt=out.start_id;
                        return 0;
                    } else if(out.id_count > 1) { /*** doing table range scan in Name space - not relying on X and Y***/
                        uint32_t x_idx;
                        rc = VCursorAddColumn(self->curs, &x_idx, "X");
                        if( rc == 0 || GetRCState(rc) == rcExists){
                            uint32_t y_idx;
                            rc = VCursorAddColumn(self->curs, &y_idx, "Y");
                            if(rc == 0 || GetRCState(rc) == rcExists){
                                spotid_t rowid;
                                for(rowid = out.start_id; rowid < out.start_id + out.id_count; rowid ++){
                                    int32_t x,y;
                                    rc = VCursorReadDirect(self->curs,rowid,x_idx,32,&x,1, &len);
                                    if(rc == 0){
                                        rc = VCursorReadDirect(self->curs,rowid,y_idx,32,&y,1, &len);
                                        if(rc == 0 && x==out.x && y==out.y){
                                            if(rslt) *rslt=rowid;
                                            return 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    rc = RC ( rcSRA, rcIndex, rcSearching, rcColumn, rcNotFound );
                }
            }
        }
    }
    return rc;
}


/* ListCol
 *  returns a list of simple column names
 *  each name represents at least one typed column
 *
 *  "names" [ out ] - return parameter for names list
 */
LIB_EXPORT rc_t CC SRATableListCol( const SRATable *self, SRANamelist **rslt ) {
    if ( self != NULL )
        return VTableListCol ( self -> vtbl, (KNamelist **)rslt );
    return RC(rcSRA, rcTable, rcListing, rcSelf, rcName);
}


/* ColDatatypes
 *  returns list of typedecls for named column
 *
 *  "col" [ IN ] - column name
 *
 *  "dflt_idx" [ OUT, NULL OKAY ] - returns the zero-based index
 *  into "typedecls" of the default datatype for the named column
 *
 *  "typedecls" [ OUT ] - list of datatypes available for named column
 */
LIB_EXPORT rc_t CC SRATableColDatatypes( const SRATable *self,
        const char *col, uint32_t *dflt_idx, SRANamelist **rslt ) {
    if ( self != NULL )
        return VTableColumnDatatypes ( self -> vtbl, col, dflt_idx, (KNamelist **)rslt );
    return RC(rcSRA, rcTable, rcListing, rcSelf, rcName);
}

rc_t SRATableColDatatype ( const SRATable *self,
        uint32_t idx, VTypedecl *type, VTypedef *def )
{
    rc_t rc;
    if ( type == NULL && def == NULL )
        rc = RC ( rcSRA, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        union { VTypedecl td; VTypedef def; } dummy;
        if ( type == NULL )
            type = & dummy . td;
        else if ( def == NULL )
            def = & dummy . def;

        if ( idx == 0 )
            rc = RC ( rcSRA, rcColumn, rcAccessing, rcSelf, rcNull );
        else if ( self == NULL )
            rc = RC ( rcSRA, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            VTypedesc desc;
            rc = VCursorDatatype ( self -> curs, idx, type, & desc );
            if ( rc == 0 )
            {
                rc = VSchemaDescribeTypedef ( VCursorGetSchema(self -> curs), def, type -> type_id );
                if ( rc == 0 )
                    return 0;
            }
        }

        memset ( type, 0, sizeof * type );
        memset ( def, 0, sizeof * def );
    }
    return rc;
}

/* MetaRevision
 *  returns current revision number
 *  where 0(zero) means tip
 */
LIB_EXPORT rc_t CC SRATableMetaRevision( const SRATable *self, uint32_t *rslt )
{
    if (self == NULL)
        return RC(rcSRA, rcTable, rcAccessing, rcSelf, rcNull);
    return KMetadataRevision (self->meta,rslt);
}


/* MaxRevision
 *  returns the maximum revision available
 */
LIB_EXPORT rc_t CC SRATableMaxMetaRevision( const SRATable *self, uint32_t *rslt )
{
    if (self == NULL)
        return RC(rcSRA, rcTable, rcAccessing, rcSelf, rcNull);
    return KMetadataMaxRevision(self->meta,rslt);
}

/* UseMetaRevision
 *  opens indicated revision of metadata
 *  all non-zero revisions are read-only
 */
LIB_EXPORT rc_t CC SRATableUseMetaRevision ( const SRATable *cself, uint32_t revision )
{
    rc_t rc;

    if ( cself == NULL )
        rc = RC ( rcSRA, rcTable, rcUpdating, rcSelf, rcNull );
    else if ( cself -> read_only == false )
        rc = RC ( rcSRA, rcTable, rcUpdating, rcMetadata, rcBusy );
    else
    {
        uint32_t cur;
        rc = KMetadataRevision ( cself -> meta, & cur );
        if ( rc == 0 && cur != revision )
        {
            SRATable *self = ( SRATable* ) cself;
            const KMetadata *meta;
            rc = KMetadataOpenRevision ( self -> meta, & meta, revision );
            if ( rc == 0 )
            {
                KMetadataRelease ( self -> meta );
                self -> meta = meta;
            }
        }
    }

    return rc;
}

/* OpenMDataNode
 *  open a metadata node
 *
 *  "node" [ OUT ] - return parameter for metadata node
 *
 *  "path" [ IN ] - simple or hierarchical NUL terminated
 *  path to node
 */
LIB_EXPORT rc_t CC SRATableOpenMDataNodeRead( const SRATable *self, 
        struct KMDataNode const **node, const char *path, ... ) {
    va_list va;
    rc_t rc;
    
    va_start(va, path);
    rc = SRATableVOpenMDataNodeRead(self, node, path, va);
    va_end(va);
    return rc;
}

LIB_EXPORT rc_t CC SRATableVOpenMDataNodeRead( const SRATable *self,
        struct KMDataNode const **rslt, const char *path, va_list args ) {
    if (self == NULL)
        return RC(rcSRA, rcTable, rcAccessing, rcSelf, rcNull);
    
    return KMetadataVOpenNodeRead(self->meta, rslt, path, args);
}

/* private */
rc_t SRATableGetIdRange ( const SRATable *self,
        uint32_t idx, spotid_t id, spotid_t *pfirst, spotid_t *plast )
{
    rc_t rc;

    if ( pfirst == NULL && plast == NULL )
        rc = RC ( rcSRA, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        spotid_t dummy;
        if ( pfirst == NULL )
            pfirst = & dummy;
        else if ( plast == NULL )
            plast = & dummy;

        if ( idx == 0 )
            rc = RC ( rcSRA, rcColumn, rcAccessing, rcSelf, rcNull );
        else if ( self == NULL )
            rc = RC ( rcSRA, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            int64_t first, last;
            rc = VCursorPageIdRange ( self -> curs, idx, id, & first, & last );
            if ( rc == 0 )
            {
                * pfirst = ( spotid_t ) first;
                * plast = ( spotid_t ) last;

                if ( ( int64_t ) * pfirst == first && ( int64_t ) * plast == last )
                    return 0;

                rc = RC ( rcSRA, rcColumn, rcAccessing, rcRange, rcExcessive );
            }
        }

        * pfirst = * plast = 0;
    }

    return rc;
}

/* semi-private for sra-dbcc */
LIB_EXPORT rc_t CC SRATableGetVTableRead( const SRATable *self, const VTable **rslt )
{
    if (rslt == NULL)
        return RC(rcSRA, rcTable, rcAccessing, rcParam, rcNull);
    
    if (self == NULL)
    {
        * rslt = NULL;
        return RC(rcSRA, rcTable, rcAccessing, rcSelf, rcNull);
    }

    *rslt = self->vtbl;
    return VTableAddRef(*rslt);
}

LIB_EXPORT rc_t CC SRATableGetKTableRead ( const SRATable *self, struct KTable const **ktbl )
{
    rc_t rc;

    if ( ktbl == NULL )
        rc = RC ( rcSRA, rcTable, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcSRA, rcTable, rcAccessing, rcSelf, rcNull );
        else
        {
            return VTableGetKTableRead ( self -> vtbl, ktbl );
        }

        * ktbl = NULL;
    }

    return rc;
}

/* Locked
 *  returns true if locked
 */
LIB_EXPORT bool CC SRATableLocked( const SRATable *self )
{
    return self ? VTableLocked(self->vtbl) : false;
}

LIB_EXPORT struct VSchema const* CC SRATableGetSchema ( struct SRATable const *self )
{
        return self ? VCursorGetSchema( self->curs ) : NULL;
}

/* sfa_filter
 *  if a name is found in list, exclude it
 */
#define DEBUG_SORT(msg) DBGMSG (DBG_SRA, DBG_FLAG(DBG_SRA_SORT), msg)

/* sfa_sort
 *  reorders list
 */
enum sfa_path_type_id
{
    sfa_not_set = -1,
    sfa_exclude,
    sfa_non_column,
    sfa_required,
    sfa_preferred,
    sfa_optional
};

#if _DEBUGGING
const char* sfa_path_type_id[] = {
    "not_set",
    "exclude",
    "non_column",
    "required",
    "preferred",
    "optional"
};
#endif

typedef struct reorder_t_struct {
    const char * path;
    uint64_t     size;
    enum sfa_path_type_id type_id;
} reorder_t;

typedef enum sfa_path_type_id (CC *sfa_path_type_func)( const char *path );

/*
union u_void_sfa_path_type_func
{
    void * func;
    sfa_path_type_func sptf;
};

static sfa_path_type_func void_to_sfa_path_type_func( void * func )
{
    union u_void_sfa_path_type_func x;
    x.func = func;
    return x.sptf;
}
*/

static
bool CC sfa_filter(const KDirectory *dir, const char *leaf, void* func)
{
    bool ret = true;
    sfa_path_type_func f = (sfa_path_type_func)func; /* void_to_sfa_path_type_func( func ); */
    enum sfa_path_type_id type = f(leaf);

    ret = type >= sfa_non_column;
    DEBUG_SORT(("%s: %s %s %s\n", __func__, leaf, sfa_path_type_id[type + 1], ret ? "keep" : "drop"));
    return ret;
}

static
bool CC sfa_filter_light(const KDirectory *dir, const char *leaf, void* func)
{
    bool ret = true;
    sfa_path_type_func f = (sfa_path_type_func)func; /* void_to_sfa_path_type_func( func ); */

    enum sfa_path_type_id type = f(leaf);

    ret = type >= sfa_non_column && type < sfa_optional;
    DEBUG_SORT(("%s: %s %s %s\n", __func__, leaf, sfa_path_type_id[type + 1], ret ? "keep" : "drop"));
    return ret;
}

#define MATCH( ptr, str ) \
    ( (memcmp(ptr, str, sizeof(str) - 2) == 0 && \
       ((ptr)[sizeof(str) - 2] == '\0' || (ptr)[sizeof(str) - 2] == '/')) ? \
        (ptr) += sizeof(str) - (((ptr)[sizeof(str) - 2] == '/') ? 1 : 2) : (const char*) 0)

static
enum sfa_path_type_id CC sfa_path_type_tbl( const char *path )
{
    /* use first character as distinguisher for match */
    switch ( path [ 0 ] )
    {
    case 'c':
        /* perhaps it's a column */
        if ( MATCH ( path, "col/" ) )
        {
            switch ( path [ 0 ] )
            {
            case 'D':
                if ( MATCH ( path, "DELETION_QV/" ) )
                    return sfa_optional;
                if ( MATCH ( path, "DELETION_TAG/" ) )
                    return sfa_optional;
                break;
            case 'H':
                if ( MATCH ( path, "HOLE_NUMBER/" ) )
                    return sfa_optional;
                if ( MATCH ( path, "HOLE_STATUS/" ) )
                    return sfa_optional;
                break;
            case 'I':
                if ( MATCH ( path, "INTENSITY/" ) )
                    return sfa_optional;
                if ( MATCH ( path, "INSERTION_QV/" ) )
                    return sfa_optional;
                break;
            case 'N':
                if ( MATCH ( path, "NAME_FMT/" ) )
                    return sfa_preferred;
                if ( MATCH ( path, "NAME/" ) )
                    return sfa_preferred;
                if ( MATCH ( path, "NOISE/" ) )
                    return sfa_optional;
                if ( MATCH ( path, "NUM_PASSES/" ) )
                    return sfa_optional;
                break;
            case 'P':
                if ( MATCH ( path, "POSITION/" ) )
                    return sfa_optional;
                if ( MATCH ( path, "PRE_BASE_FRAMES/" ) )
                    return sfa_optional;
                if ( MATCH ( path, "PULSE_INDEX/" ) )
                    return sfa_optional;
                break;
            case 'Q':
                if ( MATCH ( path, "QUALITY2/" ) )
                    return sfa_optional;
                break;
            case 'S':
                if ( MATCH ( path, "SIGNAL/" ) )
                    return sfa_optional;
                if ( MATCH ( path, "SPOT_NAME/" ) )
                    return sfa_preferred;
                if ( MATCH ( path, "SUBSTITUTION_QV/" ) )
                    return sfa_optional;
                if ( MATCH ( path, "SUBSTITUTION_TAG/" ) )
                    return sfa_optional;
                break;
            case 'W':
                if ( MATCH ( path, "WIDTH_IN_FRAMES/" ) )
                    return sfa_optional;
                break;
            case 'X':
            case 'Y':
                if ( path [ 1 ] == '/' )
                    return sfa_preferred;
                break;
            }
        }
        return sfa_required;

    case 'i':
        /* look for skey index */
        if ( MATCH ( path, "idx/skey" ) )
            if ( path [ 0 ] == 0 || strcmp ( path, ".md5" ) == 0 )
                return sfa_preferred;
        if ( MATCH ( path, "idx/fuse-" ) )
            return sfa_exclude;
        break;

    case 's':
        /* look for old skey index */
        if ( MATCH ( path, "skey" ) )
            if ( path [ 0 ] == 0 || strcmp ( path, ".md5" ) == 0 )
                return sfa_preferred;
        break;
    }
    /* anything not recognized is non-column required */
    return sfa_non_column;
}

static
enum sfa_path_type_id CC sfa_path_type_db ( const char *path )
{
    /* use first character as distinguisher for match */
    switch ( path [ 0 ] )
    {
    case 't':
        /* perhaps it's a table */
        if ( MATCH ( path, "tbl/" ) )
        {
            switch ( path [ 0 ] )
            {
            case 0:
                return sfa_non_column;
            case 'S':
                if ( MATCH ( path, "SEQUENCE/" ) )
                    return sfa_path_type_tbl(path);
                break;
            case 'C':
                if ( MATCH ( path, "CONSENSUS/" ) )
                    return sfa_path_type_tbl(path);
                break;
            case 'P':
                if ( MATCH ( path, "PRIMARY_ALIGNMENT/" ) )
                    return sfa_path_type_tbl(path);
                break;
            case 'R':
                if ( MATCH ( path, "REFERENCE/" ) )
                    return sfa_path_type_tbl(path);
                break;
            }
            /* all other tables are optional */
            return sfa_optional;
        }
    }
    /* anything not recognized is non-column required */
    return sfa_non_column;
}
#undef MATCH

static
int64_t CC sfa_path_cmp ( const void **_a, const void **_b, void * ignored )
{
    const reorder_t * a = *_a;
    const reorder_t * b = *_b;
    int64_t ret;

    DEBUG_SORT(("%s enter\t%s %s %lu \t%s %s %lu", __func__, 
                a->path, sfa_path_type_id[a->type_id + 1], a->size,
                b->path, sfa_path_type_id[b->type_id + 1], b->size));

    ret = a->type_id - b->type_id;
    if (ret == 0)
    {
        if (a->size > b->size)
            ret = 1;
        else if (a->size < b->size)
            ret = -1;
        else
            ret = strcmp (a->path, b->path);
    }
    DEBUG_SORT(("\t%d\n", ret));
    return ret;
}

typedef
struct to_nv_data_struct
{
    const KDirectory * d;
    Vector * v;
    rc_t rc;
    sfa_path_type_func path_type;
} to_nv_data;

static
void CC to_nv (void * _item, void * _data)
{
    const char* path = _item;
    to_nv_data* data = _data;
    reorder_t* obj;

    if (data->rc == 0)
    {
        obj = malloc (sizeof (*obj));
        if (obj == NULL)
            data->rc = RC (rcSRA, rcVector, rcConstructing, rcMemory, rcExhausted);
        else
        {
            rc_t rc = KDirectoryFileSize (data->d, &obj->size, "%s", path);
            if (rc == 0)
            {
                obj->path = path;
                obj->type_id = data->path_type(path);
                rc = VectorAppend (data->v, NULL, obj);
            }

            if (rc)
            {
                free (obj);
                data->rc = rc;
            }
        }
    }
}

static
void CC item_whack (void * item, void * ignored)
{
    free (item);
}

static
rc_t CC sfa_sort( const KDirectory *dir, Vector *v, sfa_path_type_func func )
{
    /* assume "v" is a vector of paths - hopefully relative to "dir" */
    Vector nv;
    to_nv_data data;
    uint32_t base;

    DEBUG_SORT(("%s enter\n", __func__));

    base = VectorStart (v);
    VectorInit (&nv, base, VectorLength (v));
    data.d = dir;
    data.v = &nv;
    data.rc = 0;
    data.path_type = func;

    VectorForEach (v, false, to_nv, &data);

    if(data.rc == 0) {
        uint32_t idx = 0;
        uint32_t limit = VectorLength (v) + base;

        VectorReorder(&nv, sfa_path_cmp, NULL);

        for (idx = base; idx < limit; ++idx) {
            const reorder_t * tmp;
            void * ignore;

            tmp = VectorGet (&nv, idx);
            data.rc = VectorSwap (v, idx + base, tmp->path, &ignore);
            if(data.rc) {
                break;
            }
        }
    }
    VectorWhack (&nv, item_whack, NULL);
    DEBUG_SORT(("%s exit %d %R\n", __func__, data.rc, data.rc));
    return data.rc;
}

static
rc_t CC sfa_sort_db( const KDirectory *dir, Vector *v )
{
    return sfa_sort(dir, v, sfa_path_type_db);
}

static
rc_t CC sfa_sort_tbl( const KDirectory *dir, Vector *v )
{
    return sfa_sort(dir, v, sfa_path_type_tbl);
}

/*
union sptd_2_void
{
    sfa_path_type_func func;
    void * ptr;
};

static void * sfa_path_type_func_to_void( sfa_path_type_func func )
{
    union sptd_2_void u;
    u.func = func;
    return u.ptr;
}
*/

/* MakeSingleFileArchive
 *  makes a single-file-archive file from an SRA table
 *
 *  contents are ordered by frequency and necessity of access
 *
 *  "lightweight" [ IN ] - when true, include only those components
 *  required for read and quality operations.
 *
 *  "ext" [OUT,NULL] - optional file name extension to use for file
 */
LIB_EXPORT rc_t CC SRATableMakeSingleFileArchive ( const SRATable *self, const KFile **sfa, bool lightweight, const char** ext)
{
    rc_t rc;

    if ( sfa == NULL )
    {
        rc = RC( rcSRA, rcFile, rcConstructing, rcParam, rcNull );
    }
    else
    {
        *sfa = NULL;
        if ( self == NULL )
        {
            rc = RC( rcSRA, rcTable, rcAccessing, rcSelf, rcNull );
        }
        else
        {
            const VDatabase *db;
            rc = VTableOpenParentRead( self->vtbl, &db );
            if ( rc == 0 && db != NULL )
            {
                const KDatabase *kdb;
                rc = VDatabaseOpenKDatabaseRead( db, &kdb );
                if ( rc == 0 )
                {
                    const KDirectory *db_dir;
                    rc = KDatabaseOpenDirectoryRead( kdb, &db_dir );
                    if ( rc == 0 )
                    {
                        rc = KDirectoryOpenTocFileRead( db_dir,
                                                        sfa,
                                                        sraAlign4Byte,
                                                        lightweight ? sfa_filter_light : sfa_filter,
                                                        ( void * )sfa_path_type_db, /* sfa_path_type_func_to_void( sfa_path_type_db ), */
                                                        sfa_sort_db );
                        KDirectoryRelease( db_dir );
                        if ( ext != NULL )
                        {
                            *ext = CSRA_EXT( lightweight );
                        }
                    }
                    KDatabaseRelease( kdb );
                }
                VDatabaseRelease( db );
            }
            else
            {
                const KTable *ktbl;
                rc = SRATableGetKTableRead( self, &ktbl );
                if ( rc == 0 )
                {
                    const KDirectory *tbl_dir;
                    rc = KTableGetDirectoryRead( ktbl, &tbl_dir );
                    if ( rc == 0 )
                    {
                        rc = KDirectoryOpenTocFileRead( tbl_dir,
                                                        sfa,
                                                        sraAlign4Byte,
                                                        lightweight ? sfa_filter_light : sfa_filter,
                                                        ( void * )sfa_path_type_tbl, /* sfa_path_type_func_to_void( sfa_path_type_tbl ), */
                                                        sfa_sort_tbl );
                        KDirectoryRelease( tbl_dir );
                        if ( ext != NULL )
                        {
                            *ext = SRA_EXT( lightweight );
                        }
                    }
                    KTableRelease(ktbl);
                }
            }
        }
    }
    return rc;
}

/* OpenAltTableRead
 *  opens a table within a database structure with a specific name
 */
SRA_EXTERN rc_t CC SRAMgrOpenAltTableRead ( const SRAMgr *self,
    const SRATable **tbl, const char *altname, const char *spec, ... )
{
    rc_t rc;
    
    va_list args;
    va_start ( args, spec );

    if ( altname == NULL || altname [ 0 ] == 0 ) {
        altname = "SEQUENCE";
    }
    rc = SRAMgrVOpenAltTableRead ( self, tbl, altname, spec, args );

    va_end ( args );
    return rc;
}
