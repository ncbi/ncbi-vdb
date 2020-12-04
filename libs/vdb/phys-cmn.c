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
#include "phys-priv.h"
#include "schema-parse.h"
#include "schema-priv.h"
#include "schema-expr.h"
#include "table-priv.h"
#include "cursor-priv.h"
#include "prod-priv.h"
#include "blob-priv.h"
#undef KONST

#include <vdb/cursor.h>
#include <kdb/column.h>
#include <kdb/meta.h>
#include <klib/symbol.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <endian.h>


#define BLOB_VALIDATION 1

/*--------------------------------------------------------------------------
 * KMDataNode
 */


/* Addr
 *  reach into node and get address
 *  returns raw pointer and node size
 */
rc_t CC KMDataNodeAddr ( const KMDataNode *self,
    const void **addr, size_t *size );


/*--------------------------------------------------------------------------
 * VPhysical
 */

/* Destroy
 */
void VPhysicalDestroy ( VPhysical *self )
{

#if PROD_REFCOUNT && ! PROD_ALL_IN_CURSOR
    PROD_TRACK_REFCOUNT(VProductionRelease, self->out);
    VProductionRelease ( self -> out, NULL );
    PROD_TRACK_REFCOUNT(VProductionRelease, self->b2p);
    VProductionRelease ( self -> b2p, NULL );
    PROD_TRACK_REFCOUNT(VProductionRelease, self->b2s);
    VProductionRelease ( self -> b2s, NULL );
    PROD_TRACK_REFCOUNT(VProductionRelease, self->in);
    VProductionRelease ( self -> in, NULL );
#endif

    KDataBufferWhack ( & self -> srow );

    SExpressionWhack ( self -> enc );

    KMDataNodeRelease ( self -> knode );

    KMetadataRelease ( self -> meta );
    KColumnRelease ( self -> kcol );

    free ( self );
}

/* Make
 */
rc_t VPhysicalMake ( VPhysical **physp, const VCursor *curs, const SPhysMember *smbr )
{
    VPhysical *phys = calloc ( 1, sizeof * phys );

    if ( phys == NULL )
        return RC ( rcVDB, rcColumn, rcConstructing, rcMemory, rcExhausted );

    phys -> curs = curs;
    phys -> smbr = smbr;

    * physp = phys;
    return 0;
}

/* FinishKColumn
 */

#if 0
#include <stdio.h>
static
rc_t pstdout ( void *ignore, const void *buffer, size_t bsize )
{
    fwrite ( buffer, 1, bsize, stdout );
    return 0;
}
#endif

static rc_t VPhysicalLazySetRange(VPhysical *self)
{
    if(self->kstart_id < 0 || self -> kstop_id < self -> kstart_id) {
	uint64_t count;
	rc_t rc = KColumnIdRange ( self -> kcol, & self -> kstart_id, & count );
	self -> kstop_id = self -> kstart_id + count - 1;
	return rc;
    }
    return 0;
}

rc_t VPhysicalFinishKColumn ( VPhysical *self, VSchema *schema, const SPhysMember *smbr )
{
    rc_t rc = 0;

    /* lazy settings .. to be set when needed */
    self -> kstart_id = 1;
    self -> kstop_id  = 0;

    if ( self -> meta == NULL )
    {
        /* bring over "no_hdr" from SPhysical */
        self -> no_hdr = ( ( const SPhysEncExpr* ) smbr -> type ) -> phys -> no_hdr;
    }
    else
    {
        /* read in metadata */
        VTypedecl td;
        rc = VPhysicalLoadMetadata ( self, & td, schema );
        /*VSchemaDump ( schema, sdmPrint, NULL, pstdout, NULL );*/
        if ( rc == 0 )
        {
            /* if member type is unknown, reset to actual type */
            if ( smbr -> td . type_id == 0 )
            {
                /* this member was introduced into cursor schema
                   by the function VCursorSupplementPhysical with
                   an unknown type - i.e. the schema compiler will
                   not recognize "any" as a member type. essentially
                   the member type setting was delayed until now */
                ( ( SPhysMember* ) smbr ) -> td = td;
            }

            /* validate that the physical column matches
               the schema member declaration type */
            else if ( ! VTypedeclCommonAncestor ( & td, schema, & smbr -> td, NULL, NULL ) )
            {
                rc = RC ( rcVDB, rcColumn, rcLoading, rcType, rcInconsistent );
                PLOGERR ( klogInt, ( klogInt, rc, "inconsistent schema and actual types for column '$(name)'"
                                     , "name=%.*s"
                                     , ( int ) smbr -> name -> name . size
                                     , smbr -> name -> name . addr ));
            }
        }
    }

    return rc;
 }

/* FinishStatic
 */
rc_t VPhysicalFinishStatic ( VPhysical *self, const VSchema *schema, const SPhysMember *smbr )
{
    /* see if data node actually exists */
    const KMDataNode *node;
    rc_t rc = KMDataNodeOpenNodeRead ( self -> knode, & node, "row" );
    if ( rc  != 0 )
        return rc;

    KMDataNodeRelease ( node );

    /* determine id range */
    rc = KMDataNodeOpenNodeRead ( self -> knode, & node, "start_id" );
    if ( rc == 0 )
    {
        /* modern static column */
        rc = KMDataNodeReadAsI64 ( node, & self -> sstart_id );
        KMDataNodeRelease ( node );
        if ( rc == 0 )
        {
            rc = KMDataNodeOpenNodeRead ( self -> knode, & node, "row_count" );
            if ( rc == 0 )
            {
                uint64_t row_count;

                rc = KMDataNodeReadAsU64 ( node, & row_count );
                KMDataNodeRelease ( node );
                assert(row_count != 0);
                self -> sstop_id = self -> sstart_id + row_count - 1;
            }
        }
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        /* get the table metadata */
        const KMetadata *tmeta = VCursorGetTable ( self -> curs ) -> meta;

        /* original static column with no range */
        self -> sstart_id = 1;

        /* work a little harder to correct this problem */
        rc = KMetadataGetSequence ( tmeta, "spot", & self -> sstop_id );
        if ( rc != 0 )
            rc = KMetadataGetSequence ( tmeta, ".", & self -> sstop_id );
        if ( rc != 0 )
        {
#if 0
            /* set to infinite range */
            self -> sstart_id <<= 63;
            self -> sstop_id = ~ self -> sstart_id;
#else
            /* set to single row */
            self -> sstart_id = 1;
            self -> sstop_id = 1;
#endif
            rc = 0;
        }
    }

    if ( rc == 0 )
    {
        size_t size;
        char type_expr [ 256 ];

        /* read column type */
        rc = KMDataNodeReadAttr ( self -> knode, "type",
            type_expr, sizeof type_expr, & size );
        if ( rc == 0 )
        {
            VTypedecl td;

            /* resolve type */
            rc = VSchemaResolveTypedecl ( schema, & td, "%s", type_expr );
            if ( rc == 0 )
            {
                /* if member type is unknown, reset to actual type */
                if ( smbr -> td . type_id == 0 )
                {
                    /* this member was introduced into cursor schema
                       by the function VCursorSupplementPhysical with
                       an unknown type - i.e. the schema compiler will
                       not recognize "any" as a member type. essentially
                       the member type setting was delayed until now */
                    ( ( SPhysMember* ) smbr ) -> td = td;
                }

                /* validate that the physical column matches
                   the schema member declaration type */
                else if ( ! VTypedeclCommonAncestor ( & td, schema, & smbr -> td, NULL, NULL ) )
                {
                    rc = RC ( rcVDB, rcColumn, rcLoading, rcType, rcInconsistent );
                    PLOGERR ( klogInt, ( klogInt, rc, "inconsistent schema and actual types for static column '$(name)'"
                               , "name=%.*s"
                               , ( int ) smbr -> name -> name . size
                               , smbr -> name -> name . addr ));
                }
            }
        }
    }

    return rc;
}


/* Open
 *  open existing columns
 *  load schema definitions
 */
rc_t VPhysicalOpenRead ( VPhysical *self, VSchema *schema, const VTable *tbl )
{
    rc_t rc;

    /* physical member name from schema */
    const SPhysMember *smbr = self -> smbr;
    const KSymbol *name = smbr -> name;

    /* mark physical as read-only */
    self -> read_only = true;

    /* open column for update */
    rc = KTableOpenColumnRead ( tbl -> ktbl, & self -> kcol,
        "%.*s", ( int ) name -> name . size - 1, name -> name . addr + 1 );
    if ( rc == 0 )
    {
        /* open its metadata */
        rc = KColumnOpenMetadataRead ( self -> kcol, & self -> meta );
        if ( rc == 0 || GetRCState ( rc ) == rcNotFound )
        {
            /* finish off common initialization */
            rc = VPhysicalFinishKColumn ( self, schema, smbr );
        }
    }

    /* it's okay if a column doesn't exist - it may exist as a static */
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = 0;
    }

    if ( rc == 0 && self -> kcol == NULL )
    {
        /* must have static col parent node */
        if ( tbl -> col_node == NULL )
            rc = RC ( rcVDB, rcTable, rcOpening, rcColumn, rcNotFound );
        else
        {
            /* look for static */
            const KMDataNode *node;
            rc = KMDataNodeOpenNodeRead ( tbl -> col_node, & node,
                "%.*s", ( int ) name -> name . size - 1, name -> name . addr + 1 );
            if ( rc == 0 )
            {
                self -> knode = node;

                /* finish off common initialization */
                rc = VPhysicalFinishStatic ( self, schema, smbr );
            }
        }
    }

    return rc;
}

/* ReadKColumn
 *  read a raw blob from kcolumn
 */
static
rc_t VPhysicalReadKColumn ( VPhysical *self, VBlob **vblob, int64_t id, uint32_t elem_bits )
{
    rc_t rc;
    VBlob *blob;
    const KColumnBlob *kblob;

    rc = VPhysicalLazySetRange(self);
    if( rc) return rc;
    /* check id against column contents */
    if ( self -> kcol == NULL ||
         id < self -> kstart_id || id > self -> kstop_id )
    {
        * vblob = NULL;
        return RC ( rcVDB, rcColumn, rcReading, rcRow, rcNotFound );
    }

#if PROD_CACHE
    /* check for recently written cache contents */
    if ( self -> b2s != NULL && self -> b2s -> cache [ 0 ] != NULL )
    {
        VProduction *b2s = self -> b2s;

        int i;
        for ( i = 0; i < PROD_CACHE; ++ i )
        {
            blob = b2s -> cache [ i ];
            if ( id >= blob -> start_id && id <= blob -> stop_id )
            {
                * vblob = blob;
                rc = VBlobAddRef ( blob );
                if ( rc != 0 )
                    return rc;

                TRACK_BLOB ( VBlobAddRef, blob );
#if PROD_CACHE > 1
                /* MRU cache */
                if ( i > 0 )
                {
                    do
                        b2s -> cache [ i ] = b2s -> cache [ i - 1 ];
                    while ( -- i > 0 );
                    b2s -> cache [ 0 ] = blob;
                }
#endif
                return 0;
            }
        }
    }
#endif

    /* find blob in KColumn
       TBD - handle potential merge/update later */
    rc = KColumnOpenBlobRead ( self -> kcol, & kblob, id );
    if ( rc == 0 )
    {
        /* get blob id range */
        uint32_t count;
        int64_t start_id;

        rc = KColumnBlobIdRange ( kblob, & start_id, & count );
        if ( rc == 0 )
        {
            /* get blob size */
            size_t num_read, remaining;

#if BLOB_VALIDATION
            KDataBuffer whole_blob;
            KColumnBlobCSData cs_data;
            bool validate_this_blob = VCursorGetTable ( self -> curs ) -> blob_validation;

            if ( rc == 0 && validate_this_blob )
            {
                rc = KColumnBlobReadAll ( kblob, & whole_blob, & cs_data, sizeof cs_data );
                /* simulate the results of next read */
                num_read = 0;
                remaining = KDataBufferBytes ( & whole_blob );
            }
            else if ( rc == 0 && !validate_this_blob )
#endif
                rc = KColumnBlobRead ( kblob, 0, NULL, 0, & num_read, & remaining );

            if ( rc == 0 )
#if BLOB_VALIDATION
                if ( validate_this_blob )
                    rc = KColumnBlobValidateBuffer ( kblob, & whole_blob, & cs_data, sizeof cs_data );
            if ( rc == 0 )
#endif

            {
                KDataBuffer buffer;

                /* fabricate "stop_id" */
                int64_t stop_id = start_id + count - 1;

                /* if the encoding was marked __no_header */
                if ( self -> no_hdr )
                    num_read = 2;

#if BLOB_VALIDATION
                /* if already have a header, just steal the buffer */
                else if ( validate_this_blob )
                {
                    buffer = whole_blob;
                    memset ( & whole_blob, 0, sizeof whole_blob );
                }

                /* test again to see if the buffer should be made */
                if ( self -> no_hdr || !validate_this_blob )
#endif
                /* create data buffer */
                rc = KDataBufferMakeBytes ( & buffer, num_read + remaining );
                if ( rc == 0 )
                {
                    /* read entire blob */
                    uint8_t *p = buffer . base;
#if BLOB_VALIDATION
                    if ( validate_this_blob )
                    {
                        if ( self -> no_hdr )
                            memmove ( & p [ num_read ], whole_blob . base, remaining );
                    }
                    else
#endif
                    rc = KColumnBlobRead ( kblob, 0,
                        & p [ num_read ], remaining, & num_read, & remaining );

                    if ( rc == 0 )
                    {
                        if ( self -> no_hdr )
                        {
                            /* create fake v1 header byte with fixed row-length:
                               000ooobb where "o" is offset ( 0 ), and
                               "b" is byte order ( always little-endian ) */
                            p [ 0 ] = ( uint8_t ) vboLittleEndian;
                            p [ 1 ] = 0;
                        }

                        /* create a proper blob */
                        rc = VBlobNew ( vblob, start_id, stop_id, "readkcolumn" );
                        TRACK_BLOB (VBlobNew, *vblob);
                        if ( rc == 0 )
                        {
                            rc = KDataBufferSub ( & buffer, & ( * vblob ) -> data, 0, UINT64_MAX );
                            assert ( rc == 0 );
                        }
                    }

                    KDataBufferWhack ( & buffer );
                }
            }

#if BLOB_VALIDATION
            if ( validate_this_blob )
                KDataBufferWhack ( & whole_blob );
#endif
        }

        KColumnBlobRelease ( kblob );
    }

    return rc;
}

/* Read
 *  read a blob from static data or decoding chain
 */
static
rc_t VPhysicalReadStatic ( VPhysical *self, VBlob **vblob, int64_t id, uint32_t elem_bits )
{
    bool reversed;
    rc_t rc = KMDataNodeByteOrder ( self -> knode, & reversed );
    if ( rc == 0 )
    {
        const KMDataNode *row;

        /* detect non-native byte-order */
        VByteOrder byte_order = vboNative;
        if ( reversed )
        {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            byte_order = vboBigEndian;
#else
            byte_order = vboLittleEndian;
#endif
        }

        /* open sub-node for row */
        rc = KMDataNodeOpenNodeRead ( self -> knode, & row, "row" );
        if ( rc == 0 )
        {
            /* get raw pointer and size in bytes */
            size_t bytes;
            const void *base;
            rc = KMDataNodeAddr ( row, & base, & bytes );
            if ( rc == 0 )
            {
                uint64_t row_bits;

                /* if there's a "size" node, read size in bits */
                const KMDataNode *size;
                rc = KMDataNodeOpenNodeRead ( self -> knode, & size, "size" );
                if ( rc == 0 )
                {
                    rc = KMDataNodeReadAsU64 ( size, & row_bits );
                    KMDataNodeRelease ( size );
                }
                else if ( GetRCState ( rc ) == rcNotFound )
                {
                    row_bits = bytes << 3;
                    rc = 0;
                }

                /* check that the row length is an integral number of elements */
                assert ( elem_bits != 0 );
                if ( rc == 0 && row_bits % elem_bits != 0 )
                    rc = RC ( rcVDB, rcColumn, rcReading, rcRow, rcCorrupt );

                /* go forth */
                if ( rc == 0 )
                {
                    KDataBuffer buffer;
                    rc = KDataBufferMake ( & buffer, elem_bits, ( uint32_t ) ( row_bits / elem_bits ) );
                    if ( rc == 0 )
                    {
                        int64_t sstart_id = self -> sstart_id;
                        int64_t sstop_id = self -> sstop_id;

                        /* copy out single row */
                        memmove ( buffer . base, base, bytes );
                        self->fixed_len = buffer.elem_count;

                        /* limit row range */
                        if ( ( ( sstop_id - sstart_id ) >> 30 ) != 0 )
                        {
                            sstart_id  =   ((id-1) &  ~0x1fffffffUL ) + 1;  /** Truncate to the nearest 1/2 billion; making sure that id is in the middle of 1 billion max window**/
                            if ( ( ( sstop_id - sstart_id ) >> 30 ) != 0 ) /** still not enough ***/
                                sstop_id = sstart_id + 0x40000000UL ; /** leave only 1 billion */
                        }

                        rc = VBlobCreateFromSingleRow ( vblob,
                            sstart_id, sstop_id, & buffer, byte_order );
                        KDataBufferWhack( &buffer );
                    }
                }
            }

            KMDataNodeRelease ( row );
        }
    }

    return rc;
}

rc_t VPhysicalReadBlob ( VPhysical *self, VBlob **vblob, int64_t id, uint32_t elem_bits )
{
    rc_t rc;

    /* check for hit on static guy */
    if ( self -> knode != NULL &&
         id >= self -> sstart_id && id <= self -> sstop_id )
    {
        return VPhysicalReadStatic ( self, vblob, id, elem_bits );
    }

    /* need to read from kcolumn path */
    rc = VProductionReadBlob ( self -> b2p, vblob, & id , 1, NULL);
	if ( rc == 0 )
    {
	    if((*vblob)->pm==NULL)
        {
            rc = PageMapProcessGetPagemap( VCursorPageMapProcessRequest ( self -> curs ), &(*vblob)->pm);
	    }
    }

	return rc;
}


/*--------------------------------------------------------------------------
 * VPhysicalProd
 */

rc_t VPhysicalProdMake ( VProduction **prodp, Vector *owned,
    struct VCursor *curs, VPhysical *phys, int sub, const char *name,
    const VFormatdecl *fd, const VTypedesc *desc )
{
#define PHYSPROD_INDEX_OFFSET 1000000000
    VPhysicalProd *prod;
    rc_t rc = VProductionMake ( prodp, owned, sizeof * prod,
        prodPhysical, sub, name, fd, desc, NULL, chainDecoding );
    if ( rc == 0 )
    {
        prod = ( VPhysicalProd* ) * prodp;

        /* this class only knows how to redirect messages to VPhysical */
        prod -> phys = phys;
    	if(sub == prodPhysicalOut)
        {
		    (*prodp) -> cctx.cache   = VCursorGetBlobMruCache ( curs );
		    (*prodp) -> cctx.col_idx = PHYSPROD_INDEX_OFFSET + VCursorIncrementPhysicalProductionCount ( curs );
	    }
    }
    return rc;
}


void VPhysicalProdDestroy ( VPhysicalProd *self )
{
}


/* Read
 */
rc_t VPhysicalProdRead ( VPhysicalProd *self, VBlob **vblob, int64_t id, uint32_t cnt )
{
    uint32_t elem_bits;

    if ( self == NULL )
        return RC ( rcVDB, rcProduction, rcReading, rcSelf, rcNull );
    if ( self -> phys == NULL )
        return RC ( rcVDB, rcColumn, rcReading, rcSelf, rcNull );

    elem_bits = VTypedescSizeof ( & self -> dad . desc );

    switch ( self -> dad . sub )
    {
    case prodPhysicalOut:
        return VPhysicalRead ( self -> phys, vblob, id, cnt, elem_bits );
    case prodPhysicalKCol:
        return VPhysicalReadKColumn ( self -> phys, vblob, id, elem_bits );
    }

    return RC ( rcVDB, rcProduction, rcReading, rcType, rcInvalid );
}

uint32_t VPhysicalProdFixedRowLength ( const VPhysicalProd *Self, int64_t row_id ) {
    const VPhysical *self;

    if ( Self == NULL )
        return 0;
    self = Self->phys;
    if ( self == NULL )
        return 0;
    return self->fixed_len;
}

rc_t VPhysicalProdColumnIdRange ( const VPhysicalProd *Self,
                               int64_t *first, int64_t *last )
{
    const VPhysical *self;

    if ( Self == NULL )
        return RC ( rcVDB, rcProduction, rcReading, rcSelf, rcNull );

    self = Self->phys;
    if ( self == NULL )
        return RC ( rcVDB, rcColumn, rcReading, rcSelf, rcNull );

    if ( self -> knode != NULL )
    {
        *first = self->sstart_id;
        *last  = self->sstop_id;
        return 0;
    }

    if ( self -> kcol != NULL )
    {
        rc_t rc = VPhysicalLazySetRange ( ( VPhysical * ) self );
        if ( rc == 0 )
        {
            *first = self->kstart_id;
            *last  = self->kstop_id;
        }
        return rc;
    }

    return RC ( rcVDB, rcColumn, rcReading, rcRange, rcEmpty );
}


/* IsStatic
 *  is this a static column
 */
rc_t VPhysicalIsStatic ( const VPhysical *self, bool *is_static )
{
    assert ( is_static != NULL );

    if ( self == NULL )
    {
        * is_static = false;
        return RC ( rcVDB, rcColumn, rcAccessing, rcSelf, rcNull );
    }

    * is_static = self -> knode != NULL;
    return 0;
}

/* GetKColumn
 *  try to get a KColumn,
 *  and if that fails, indicate whether the column is static
 */
rc_t VPhysicalGetKColumn ( const VPhysical * self, struct KColumn ** kcol, bool * is_static )
{
    assert ( kcol != NULL );
    assert ( is_static != NULL );

    if ( self == NULL )
    {
        * kcol = NULL;
        * is_static = false;
        return RC ( rcVDB, rcColumn, rcAccessing, rcSelf, rcNull );
    }

    if ( self -> kcol != NULL )
    {
        rc_t rc = KColumnAddRef ( self -> kcol );
        if ( rc == 0 )
            * kcol = ( KColumn * ) self -> kcol;
        return rc;
    }

    if ( self -> knode != NULL )
    {
        * is_static = true;
        return SILENT_RC ( rcVDB, rcColumn, rcAccessing, rcType, rcIncorrect );
    }

    return RC ( rcVDB, rcColumn, rcAccessing, rcColumn, rcNotOpen );
}
