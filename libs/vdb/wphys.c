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
#include <os-native.h>

#define KONST
#define SKONST
#include "phys-priv.h"
#include "schema-parse.h"
#include "schema-priv.h"
#include "schema-expr.h"
#include "table-priv.h"
#include "cursor-priv.h"
#include "prod-priv.h"
#include "prod-expr.h"
#include "blob-priv.h"
#include "page-map.h"
#undef KONST
#undef SKONST

#include <vdb/vdb-priv.h>
#include <vdb/cursor.h>
#include <kdb/manager.h>
#include <kdb/table.h>
#include <kdb/column.h>
#include <kdb/meta.h>
#include <klib/symbol.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <bitstr.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


#if VCURSOR_WRITE_MODES_SUPPORTED
#error "only kcmInsert and kcmReplace modes supported"
#else
#define WHOLE_REPLACE_UPDATE 1
#endif

/*--------------------------------------------------------------------------
 * VPhysical
 */

static
rc_t VPhysicalConvertStatic ( VPhysical *self );


/* Whack
 */
void CC VPhysicalWhack ( void *item, void *ignore )
{
    VPhysical *self = item;
    if ( self > FAILED_PHYSICAL )
    {
#if PROD_CACHE
        /* detect close of static column */
        if ( ! self -> read_only && self -> knode != NULL )
        {
            /* if the column is fairly large for metadata */
            if ( KDataBufferBytes ( & self -> srow ) > 64 * 1024 )
                /* convert it to a single blob */
                VPhysicalConvertStatic ( self );
        }
#endif
        if ( ! self -> read_only )
        {
            rc_t rc;
            VCursor *curs = self -> curs;
            VTable *tbl = ( VTable * ) VCursorGetTable ( curs );

            const String *name = & self -> smbr -> name -> name;

            KMDataNode *col_node = tbl -> col_node;

            /* create rename strings */
            char buff [ 300 ];
            char *oldname, *colname, *tmpname = buff;
            int sz = snprintf ( buff, sizeof buff / 3, "%.*s.tmp",
                ( int ) name -> size - 1, name -> addr + 1 );
            if ( sz < 0 || sz > sizeof buff / 3 )
            {
                tmpname = malloc ( 3 * 4 * 1024 );
                if ( tmpname == NULL )
                {
                    rc = RC ( rcVDB, rcColumn, rcClosing, rcMemory, rcInsufficient );
                    PLOGERR ( klogSys, ( klogInt, rc, "failed to close column '$(colname)'",
                        "colname=%.*s", ( int ) name -> size - 1, name -> addr + 1 ));
                    return;
                }

                sz = snprintf ( tmpname, 4 * 1024, "%.*s.tmp",
                    ( int ) name -> size - 1, name -> addr + 1 );
                assert ( sz > 0 && sz < 4 * 1024 );
            }
            oldname = & tmpname [ ++ sz ];
            colname = & oldname [ sz ];
            sprintf ( oldname, "%.*s.old", ( int ) name -> size - 1, name -> addr + 1 );
            sprintf ( colname, "%.*s", ( int ) name -> size - 1, name -> addr + 1 );

            /* close and rename static column */
            if ( self -> knode != NULL )
            {
                rc = KMDataNodeRelease ( self -> knode );
                if ( rc != 0 )
                {
                    PLOGERR ( klogInt, ( klogInt, rc, "failed to release static node when closing column '$(colname)'",
                                         "colname=%s", colname ));
                }
                else
                {
                    self -> knode = NULL;

                    assert ( col_node != NULL );
                    rc = KMDataNodeDropChild ( col_node, "%s", colname );
                    if ( rc != 0 && GetRCState ( rc ) != rcNotFound )
                    {
                        PLOGERR ( klogInt, ( klogInt, rc, "failed to drop previous static node when closing column '$(colname)'",
                                             "colname=%s", colname ));
                    }
                    else
                    {
                        rc = KMDataNodeRenameChild ( col_node, tmpname, colname );
                        if ( rc != 0 )
                        {
                            PLOGERR ( klogInt, ( klogInt, rc, "failed to rename static node when closing column '$(colname)'",
                                                 "colname=%s", colname ));
                        }
                        else
                        {
                            KTable *ktbl;
                            rc = VTableOpenKTableUpdate ( tbl, & ktbl );
                            if ( rc == 0 )
                            {
                                rc = KTableUnlock ( ktbl, kptColumn, "%s", colname );
                                if ( rc == 0 || GetRCState ( rc ) == rcUnlocked )
                                    rc = KTableDropColumn ( ktbl, "%s", colname );
                                KTableRelease ( ktbl );
                                rc = 0;
                            }
                        }
                    }
                }
            }
            else if ( self -> kcol != NULL )
            {
                KMDataNode *node;

                /* write fixed length to column metadata */
                if ( self -> fixed_len != 0 )
                {
                    rc = KMetadataOpenNodeUpdate ( self -> meta, & node, "row-len" );
                    if ( rc != 0 )
                    {
                        PLOGERR ( klogInt, ( klogInt, rc, "failed to open '$(node)' node when closing column '$(colname)'",
                            "node=row-len,colname=%s", colname ));
                    }
                    else
                    {
                        if ( self -> fixed_len < 0x100 )
                        {
                            uint8_t u8 = ( uint8_t ) self -> fixed_len;
                            rc = KMDataNodeWriteB8 ( node, & u8 );
                        }
                        else if ( self -> fixed_len < 0x10000 )
                        {
                            uint16_t u16 = ( uint16_t ) self -> fixed_len;
                            rc = KMDataNodeWriteB16 ( node, & u16 );
                        }
                        else
                        {
                            rc = KMDataNodeWriteB32 ( node, & self -> fixed_len );
                        }
                        if ( rc != 0 )
                        {
                            PLOGERR ( klogInt, ( klogInt, rc, "failed to update '$(node)' node when closing column '$(colname)'",
                                "node=row-len,colname=%s", colname ));
                        }

                        KMDataNodeRelease ( node );
                    }
                }
                else
                {
                    rc = KMetadataOpenNodeUpdate ( self -> meta, & node, NULL );
                    if ( rc != 0 )
                    {
                        PLOGERR ( klogInt, ( klogInt, rc, "failed to open '$(node)' node when closing column '$(colname)'",
                            "node=/,colname=%s", colname ));
                    }
                    else
                    {
                        rc = KMDataNodeDropChild ( node, "row-len" );
                        if ( rc == 0 && GetRCState ( rc ) != rcNotFound )
                        {
                            PLOGERR ( klogInt, ( klogInt, rc, "failed to update '$(node)' node when closing column '$(colname)'",
                                "node=row-len,colname=%s", colname ));
                        }

                        KMDataNodeRelease ( node );
                    }
                }

                /* close column */
                rc = KMetadataRelease ( self -> meta );
                if ( rc != 0 )
                    PLOGERR ( klogInt, ( klogInt, rc, "failed to release column '$(colname)'", "colname=%s", colname ));
                else
                {
                    self -> meta = NULL;
                    rc = KColumnRelease ( self -> kcol );
                    if ( rc != 0 )
                        PLOGERR ( klogInt, ( klogInt, rc, "failed to release column '$(colname)'", "colname=%s", colname ));
                    else
                    {
                        KTable *ktbl;

                        self -> kcol = NULL;

                        /* access KTable */
                        rc = VTableOpenKTableUpdate ( tbl, & ktbl );
                        if ( rc != 0 )
                        {
                            PLOGERR ( klogInt, ( klogInt, rc, "failed to access physical table when closing column '$(colname)'",
                                                 "colname=%s", colname ));
                        }
                        else
                        {
                            bool has_old;

                            /* unlock existing */
                            rc = KTableUnlock ( ktbl, kptColumn, "%s", colname );
                            if ( rc == 0 || GetRCState ( rc ) == rcUnlocked )
                            {
                                has_old = true;

                                /* rename existing */
                                rc = KTableRenameColumn ( ktbl, true, colname, oldname );
                                if ( rc != 0 )
                                {
                                    PLOGERR ( klogSys, ( klogInt, rc, "failed to rename previous column when closing '$(colname)'",
                                                         "colname=%s", colname ));
                                }
                            }
                            else if ( GetRCState ( rc ) == rcNotFound )
                            {
                                has_old = false;
                                rc = 0;
                            }
                            if ( rc == 0 )
                            {
                                /* rename new column */
                                rc = KTableRenameColumn ( ktbl, true, tmpname, colname );
                                if ( rc != 0 )
                                {
                                    PLOGERR ( klogSys, ( klogInt, rc, "failed to rename previous column when closing '$(colname)'",
                                                         "colname=%s", colname ));
                                    if ( has_old )
                                    {
                                        rc = KTableRenameColumn ( ktbl, true, oldname, colname );
                                        if ( rc != 0 )
                                        {
                                            PLOGERR ( klogSys, ( klogInt, rc, "failed to restore previous column when closing '$(colname)'",
                                                                 "colname=%s", colname ));
                                        }
                                    }
                                }
                                else
                                {
                                    if ( has_old )
                                    {
                                        /* drop old column */
                                        rc = KTableDropColumn ( ktbl, "%s", oldname );
                                        if ( rc != 0 )
                                        {
                                            PLOGERR ( klogWarn, ( klogInt, rc, "preexisting column was not dropped when closing '$(colname)'",
                                                                  "colname=%s", colname ));
                                        }
                                    }

                                    KMDataNodeDropChild ( col_node, "%s", colname );
                                }
                            }

                            KTableRelease ( ktbl );
                        }
                    }
                }
            }

            if ( tmpname != buff )
                free ( tmpname );
        }

        VPhysicalDestroy ( self );
    }
}

/* Open
 *  open existing columns
 *  load schema definitions
 */
rc_t VPhysicalOpenWrite ( VPhysical *self, VSchema *schema, const VTable *tbl )
{
#if WHOLE_REPLACE_UPDATE
    return 0;
#else
    /* physical member name from schema */
    const SPhysMember *smbr = self -> smbr;
    const KSymbol *name = smbr -> name;

    /* open column for update */
    rc_t rc = KTableOpenColumnUpdate ( tbl -> ktbl, & self -> kcol,
        "%.*s", ( int ) name -> name . size - 1, name -> name . addr + 1 );
    if ( rc == 0 )
    {
        /* open its metadata */
        rc = KColumnOpenMetadataUpdate ( self -> kcol, & self -> meta );
        if ( rc == 0 )
        {
            /* finish off common initialization */
            rc = VPhysicalFinishKColumn ( self, schema, smbr );
            if(rc==0) rc = VPhysicalLazySetRange(self);
        }
    }

    /* it's okay if a column doesn't exist -
       it'll either be created or will exist as a static */
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = 0;
    }

    if ( rc == 0 && self -> kcol == NULL )
    {
        /* look for static */
        const KMDataNode *node;
        assert ( tbl -> col_node != NULL );
        rc = KMDataNodeOpenNodeRead ( tbl -> col_node, & node,
            "%.*s", ( int ) name -> name . size - 1, name -> name . addr + 1 );
        if ( rc == 0 )
        {
            /* reopen for update */
            KMDataNodeRelease ( node );
            rc = KMDataNodeOpenNodeUpdate ( tbl -> col_node, & self -> knode,
                "%.*s", ( int ) name -> name . size - 1, name -> name . addr + 1 );
            if ( rc == 0 )
            {
                /* finish off common initialization */
                rc = VPhysicalFinishStatic ( self, schema, smbr );
            }
        }
        else if ( GetRCState ( rc ) == rcNotFound )
        {
            rc = 0;
        }
    }

    return rc;
#endif
}

static
rc_t VPhysicalCreateStatic ( VPhysical *self, const VBlob *vblob )
{
    rc_t rc;
    const String *name;

    VCursor *curs = self -> curs;
    const VTable *tbl = VCursorGetTable ( curs );

    KMDataNode *col_node = tbl -> col_node;
    assert ( col_node != NULL );

    /* create node */
    name = & self -> smbr -> name -> name;
    rc = KMDataNodeOpenNodeUpdate ( col_node, & self -> knode,
        "%.*s.tmp", ( int ) name -> size - 1, name -> addr + 1 );
    if ( rc == 0 )
    {
        /* type */
        char typedecl [ 256 ];
        rc = VTypedeclToText ( & self -> smbr -> td,
            VCursorGetSchema ( curs ), typedecl, sizeof typedecl );
        if ( rc == 0 )
            rc = KMDataNodeWriteAttr ( self -> knode, "type", typedecl );
        if ( rc == 0 )
        {
            /* open row */
            KMDataNode *row;
            rc = KMDataNodeOpenNodeUpdate ( self -> knode, & row, "row" );
            if ( rc == 0 )
            {
                /* keep a copy of the original */
                rc = KDataBufferSub ( & vblob -> data, & self -> srow, 0, UINT64_MAX );
                if ( rc == 0 )
                {
                    /* total bits in static row */
                    uint64_t row_bits = KDataBufferBits ( & vblob -> data );

                    /* check for need to reverse byte order */
                    bool reverse;
                    rc = KMDataNodeByteOrder ( row, & reverse );
                    if ( rc == 0 )
                    {
                        KDataBuffer data;

                        /* if the output metadata file is in native byte order */
                        if ( ! reverse )
                            rc = KDataBufferSub ( & vblob -> data, & data, 0, UINT64_MAX );
                        else
                        {
                            /* find the datatype for static column */
                            const SDatatype *dt = VSchemaFindTypeid ( VCursorGetSchema ( curs ),
                                self -> smbr -> td . type_id );
                            assert ( dt != NULL );

                            /* if the type does not need swapping */
                            if ( dt -> byte_swap == NULL )
                                rc = KDataBufferSub ( & vblob -> data, & data, 0, UINT64_MAX );
                            else
                            {
                                /* swap into a writable buffer */
                                rc = KDataBufferMake ( & data, vblob -> data . elem_bits, vblob -> data . elem_count );
                                if ( rc == 0 )
                                {
                                    ( * dt -> byte_swap ) ( data . base,
                                        vblob -> data . base, ( uint32_t ) ( row_bits / dt -> size ) );
                                }
                            }
                        }
                        if ( rc == 0 )
                        {
                            /* write row */
                            rc = KMDataNodeWrite ( row, data . base, ( size_t ) ( ( row_bits + 7 ) >> 3 ) );
                            if ( rc == 0 )
                            {
                                /* if row-bits are not integral bytes */
                                if ( ( row_bits & 7 ) != 0 )
                                {
                                    /* record them in a size node */
                                    KMDataNode *size;
                                    rc = KMDataNodeOpenNodeUpdate ( self -> knode, & size, "size" );
                                    if ( rc == 0 )
                                    {
                                        rc = KMDataNodeWriteB64 ( size, & row_bits );
                                        KMDataNodeRelease ( size );
                                    }
                                }
                            }

                            KDataBufferWhack ( & data );
                        }
                    }
                }

                KMDataNodeRelease ( row );
            }
        }

        if ( rc != 0 )
        {
            KMDataNodeRelease ( self -> knode ), self -> knode = NULL;
            KMDataNodeDropChild ( col_node, "%.*s", ( int ) name -> size - 1, name -> addr + 1 );
            KDataBufferWhack ( & self -> srow );
        }
        else
        {
            self -> sstart_id = vblob -> start_id;
            self -> sstop_id = vblob -> stop_id;
            self -> fixed_len = PageMapGetIdxRowInfo ( vblob -> pm, 0, NULL, NULL );
        }
    }

    return rc;
}

static
rc_t VPhysicalCreateKColumn ( VPhysical *self )
{
    rc_t rc;
    VCursor *curs = self -> curs;
    const SPhysMember *smbr = self -> smbr;
    const VTable *tbl = VCursorGetTable ( curs );
    const String *name = & smbr -> name -> name;
    const VSchema * schema = VCursorGetSchema ( curs );

    /* check for static designation */
    if ( smbr -> stat )
    {
        rc = RC ( rcVDB, rcColumn, rcCreating, rcConstraint, rcViolated );
        return rc;
    }

    /* create physical column */
    rc = KTableCreateColumn ( tbl -> ktbl, & self -> kcol,
        ( KCreateMode ) tbl -> cmode, ( KChecksum ) tbl -> checksum, tbl -> pgsize,
        "%.*s.tmp", ( int ) name -> size - 1, name -> addr + 1 );
    if ( rc == 0 )
    {
        /* create its metadata */
        rc = KColumnOpenMetadataUpdate ( self -> kcol, & self -> meta );
        if ( rc == 0 )
        {
            /* print typedecl */
            char buffer [ 256 ];
            rc = VTypedeclToText ( & smbr -> td,
                schema, buffer, sizeof buffer );
            if ( rc == 0 )
            {
                /* create schema node */
                KMDataNode *schema_node;
                rc = KMetadataOpenNodeUpdate ( self -> meta, & schema_node, "schema" );
                if ( rc == 0 )
                {
                    /* write column type */
                    rc = KMDataNodeWriteAttr ( schema_node, "type", buffer );
                    if ( rc == 0 )
                    {
                        /* going to write some schema */
                        if ( smbr -> type == NULL )
                        {
                            /* buffer already contains datatype */
                            if ( smbr -> td . dim != 1 )
                            {
                                /* get just the typename */
                                /* these statements do nothing! gcc points it out in a warning.
                                VTypedecl td = smbr -> td;
                                td . dim = 1;
                                */
                                rc = VTypedeclToText ( & smbr -> td,
                                    schema, buffer, sizeof buffer );
                            }
                        }
                        else
                        {
                            size_t num_writ;

                            /* get physical type expression */
                            rc = VSchemaToText ( schema,
                                buffer, sizeof buffer, & num_writ, "%E", smbr -> type );
                            if ( rc == 0 )
                            {
                                buffer [ num_writ ] = 0;
                                rc = KMDataNodeWriteAttr ( schema_node, "expr", buffer );
                                if ( rc == 0 )
                                {
                                    const SPhysical *sphys = ( ( const SPhysEncExpr* ) smbr -> type ) -> phys;
                                    rc = VSchemaToText ( schema,
                                        buffer, sizeof buffer, & num_writ, "%N%V",
                                        sphys -> name, sphys -> version );
                                    if ( rc == 0 )
                                        buffer [ num_writ ] = 0;
                                }
                            }
                        }
                    }
                    if ( rc == 0 )
                    {
                        rc = VSchemaDump ( schema, sdmCompact, buffer,
                           ( rc_t ( CC * ) ( void*, const void*, size_t ) ) KMDataNodeAppend, schema_node );
                        if ( rc == 0 )
                        {
                            /* mark it empty */
                            self -> kstart_id = 1;
                            self -> kstop_id = 0;
                        }
                    }
                }

                KMDataNodeRelease ( schema_node );
            }
        }
    }

    return rc;
}

static
rc_t VPhysicalWriteKColumn ( VPhysical *self, const VBlob *vblob )
{
    KColumnBlob *kblob;
    rc_t rc = KColumnCreateBlob ( self -> kcol, & kblob );
    if ( rc == 0 )
    {
        /* for now, row counts are 32-bit */
        uint32_t count = (uint32_t)(vblob -> stop_id - vblob -> start_id + 1);
        rc = KColumnBlobAssignRange ( kblob, vblob -> start_id, count );
        if ( rc == 0 )
        {
            rc = KColumnBlobAppend ( kblob, vblob -> data . base, KDataBufferBytes ( & vblob -> data ) );
            if ( rc == 0 )
            {
                rc = KColumnBlobCommit ( kblob );
                if ( rc == 0 )
                {
                    /* get the fixed row-length of this blob */
                    uint32_t blob_fixed = VBlobFixedRowLength ( vblob );

                    /* detect if the column is truly fixed row length */
                    if ( self -> kstart_id > self -> kstop_id )
                        self -> fixed_len = blob_fixed;
                    else if ( blob_fixed != self -> fixed_len )
                        self -> fixed_len = 0;

                    /* incorporate id range into our view of the world */
                    if ( vblob -> start_id < self -> kstart_id )
                        self -> kstart_id = vblob -> start_id;
                    if ( vblob -> stop_id > self -> kstop_id )
                        self -> kstop_id = vblob -> stop_id;
                }
            }
        }

        KColumnBlobRelease ( kblob );
    }
    return rc;
}

static
rc_t VPhysicalConvertStatic ( VPhysical *self )
{
#if ! PROD_CACHE
    return RC ( rcVDB, rcColumn, rcConverting, rcFunction, rcUnsupported );
#else
    rc_t rc = VPhysicalCreateKColumn ( self );
    if ( rc == 0 )
    {
        int64_t sstart_id, sstop_id;

        /* save incoming blob cache on stack */
        VBlob *cache [ PROD_CACHE ];
        assert ( sizeof cache == sizeof self -> in -> cache );
        memmove ( cache, self -> in -> cache, sizeof cache );

        /* set incoming blob cache to empty */
        memset ( self -> in -> cache, 0, sizeof self -> in -> cache );

        for ( sstart_id = self -> sstart_id;
              rc == 0 && sstart_id <= self -> sstop_id;
              sstart_id = sstop_id + 1 )
        {
            /* 3G rows */
            sstop_id = sstart_id + 0xC0000000;
            assert ( sstop_id > sstart_id );

            /* limit to actual end */
            if ( sstop_id > self -> sstop_id )
                sstop_id = self -> sstop_id;

            /* don't create tiny additional blobs */
            else if ( ( self -> sstop_id - sstop_id ) < 1000 )
                sstop_id = self -> sstop_id;

            /* recreate a VBlob from static that can be sent through encoding */
            rc = VBlobCreateFromSingleRow ( & self -> in -> cache [ 0 ],
                sstart_id, sstop_id, & self -> srow, vboNative );
            if ( rc == 0 )
            {
                /* get an encoded version of static blob */
                VBlob *vblob;
                int64_t rowId = sstart_id;
                rc = VProductionReadBlob ( self -> b2s, & vblob, & rowId, 1,NULL );
                if ( rc == 0 )
                {
                    /* write encoded blob to physical */
                    rc = VPhysicalWriteKColumn ( self, vblob );

                    /* in all events, release blob */
                    TRACK_BLOB ( VBlobRelease, vblob );
                    ( void ) VBlobRelease ( vblob );
                }

                /* release the blob from static */
                TRACK_BLOB ( VBlobRelease, self -> in -> cache [ 0 ] );
                ( void ) VBlobRelease ( self -> in -> cache [ 0 ] );
            }
        }

        /* restore cache from stack */
        memmove ( self -> in -> cache, cache, sizeof self -> in -> cache );

        if ( rc == 0 )
        {
            const String *name;
            VCursor *curs = self -> curs;
            const VTable *tbl = VCursorGetTable ( curs );

            /* tear down static */
            KMDataNodeRelease ( self -> knode );
            self -> knode = NULL;

            /* delete column from table */
            name = & self -> smbr -> name -> name;
            assert ( tbl -> col_node != NULL );
            rc = KMDataNodeDropChild ( tbl -> col_node,
                "%.*s.tmp", ( int ) name -> size - 1, name -> addr + 1 );
        }
    }

    return rc;
#endif
}

static
rc_t KMDataNodeWriteId ( KMDataNode *self, int64_t id )
{
    int32_t i32;

    if ( id >= -128 && id < 128 )
    {
        int8_t i8 = ( int8_t ) id;
        return KMDataNodeWriteB8 ( self, & i8 );
    }
    if ( id >= -32768 && id < 32768 )
    {
        int16_t i16 = ( int16_t ) id;
        return KMDataNodeWriteB16 ( self, & i16 );
    }

    i32 = ( int32_t ) id;
    if ( ( int64_t ) i32 == id )
        return KMDataNodeWriteB32 ( self, & i32 );

    return KMDataNodeWriteB64 ( self, & id );
}

static
rc_t KMDataNodeWriteCount ( KMDataNode *self, uint64_t count )
{
    if ( count < 0x100 )
    {
        uint8_t i8 = ( uint8_t ) count;
        return KMDataNodeWriteB8 ( self, & i8 );
    }
    if ( count < 0x10000 )
    {
        uint16_t i16 = ( uint16_t ) count;
        return KMDataNodeWriteB16 ( self, & i16 );
    }
    if ( ( count >> 32 ) == 0 )
    {
        uint32_t i32 = ( uint32_t ) count;
        return KMDataNodeWriteB32 ( self, & i32 );
    }
    return KMDataNodeWriteB64 ( self, & count );
}

static
rc_t VPhysicalSetStaticId ( VPhysical *self )
{
    KMDataNode *node;
    rc_t rc = KMDataNodeOpenNodeUpdate ( self -> knode, & node, "start_id" );
    if ( rc == 0 )
    {
        rc = KMDataNodeWriteId ( node, self -> sstart_id );
        KMDataNodeRelease ( node );

        if ( rc == 0 )
        {
            rc = KMDataNodeOpenNodeUpdate ( self -> knode, & node, "row_count" );
            if ( rc == 0 )
            {
                rc = KMDataNodeWriteCount ( node, self -> sstop_id - self -> sstart_id + 1 );
                KMDataNodeRelease ( node );
            }
        }
    }
    return rc;
}

static
rc_t VPhysicalWrite ( VPhysical *self, int64_t id, uint32_t cnt )
{
    /* read from page space */
    VBlob *vblob;
    rc_t rc = VProductionReadBlob ( self -> in, & vblob, & id , cnt,NULL);
    if ( rc == 0 )
    {
        /* test for single row in blob */
        assert ( vblob != NULL );
        if ( VBlobIsSingleRow ( vblob ) )
        {
            /* new column */
            if ( self -> knode == NULL && self -> kcol == NULL )
            {
                rc = VPhysicalCreateStatic ( self, vblob );
                TRACK_BLOB ( VBlobRelease, vblob );
                ( void ) VBlobRelease ( vblob );
                if ( rc == 0 )
                    rc = VPhysicalSetStaticId ( self );
                return rc;
            }

            /* existing static column */
            if ( self -> knode != NULL )
            {
                /* not allowing both to be active at the same time */
                assert ( self -> kcol == NULL );

                /* overlapping or adjacent id ranges */
                assert ( vblob -> start_id <= vblob -> stop_id );
                if ( vblob -> stop_id + 1 >= self -> sstart_id &&
                     vblob -> start_id <= self -> sstop_id + 1 )
                {
                    /* compare lengths */
                    if ( self -> fixed_len == PageMapGetIdxRowInfo ( vblob -> pm, 0, NULL, NULL ) )
                    {
                        /* compare bits */
                        assert ( KDataBufferBits ( & self -> srow ) == KDataBufferBits ( & vblob -> data ) );
                        if ( bitcmp ( self -> srow . base, self -> srow . bit_offset,
                                      vblob -> data . base, vblob -> data . bit_offset,
                                      KDataBufferBits ( & self -> srow ) ) == 0 )
                        {
                            /* it's fine */
                            if ( vblob -> start_id < self -> sstart_id )
                                self -> sstart_id = vblob -> start_id;
                            if ( vblob -> stop_id > self -> sstop_id )
                                self -> sstop_id = vblob -> stop_id;

                            TRACK_BLOB ( VBlobRelease, vblob );
                            ( void ) VBlobRelease ( vblob );

                            return VPhysicalSetStaticId ( self );
                        }
                    }
                }
            }
        }

        /* At this point we can no longer be a static row:
         * the current blob might have been more than a single row, or
         * it might have been unable to extend range of static as a single row */
        if ( self -> knode != NULL )
            rc = VPhysicalConvertStatic ( self );

        /* need to write to KColumn */
        if ( rc == 0 )
        {
            /* not allowing both knode and kcol to be active at the same time */
            assert ( self -> knode == NULL );

            /* create KColumn if necessary */
            if ( self -> kcol == NULL )
                rc = VPhysicalCreateKColumn ( self );

            /* pull through encoding */
            if ( rc == 0 )
            {
                TRACK_BLOB ( VBlobRelease, vblob );
                ( void ) VBlobRelease ( vblob );
                rc = VProductionReadBlob ( self -> b2s, & vblob, & id, cnt,NULL );
                if ( rc == 0 )
                {
                    /* write encoded blob to physical */
                    rc = VPhysicalWriteKColumn ( self, vblob );
                }
            }
        }

        TRACK_BLOB ( VBlobRelease, vblob );
        ( void ) VBlobRelease ( vblob );
    }

    return rc;
}

/* Read
 *  get the blob
 */
rc_t VPhysicalRead ( VPhysical *self, VBlob **vblob, int64_t id, uint32_t cnt, uint32_t elem_bits )
{
    /* use this as the chance to read from input */
    if ( self -> in != NULL )
    {
        rc_t rc = VPhysicalWrite ( self, id, cnt );
        if ( rc != 0 )
        {
            * vblob = NULL;
            return rc;
        }
    }

    /* now let read-side do its work */
    return VPhysicalReadBlob ( self, vblob, id, elem_bits );
}
