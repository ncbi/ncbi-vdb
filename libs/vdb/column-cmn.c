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


#define USE_KURT 1


#include <vdb/extern.h>

#define KONST const
#include "column-priv.h"
#include "dbmgr-priv.h"
#include "schema-priv.h"
#include "prod-priv.h"
#include "blob-priv.h"
#include "page-map.h"
#undef KONST

#include <vdb/manager.h>
#include <kdb/column.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * VColumn
 */

/* Whack
 */
void VColumnDestroy ( VColumn *self )
{
#if PROD_REFCOUNT && ! PROD_ALL_IN_CURSOR
    PROD_TRACK_REFCOUNT (VProductionRelease,self->in);
    VProductionRelease ( self -> in, NULL );
#endif
#if USE_KURT
    VBlobRelease ( self -> cache );
#endif
    VSchemaSever ( self -> schema );
}


/* Cmp
 * Sort
 */
int CC VColumnCmp ( const void *item, const void *n )
{
    const VColumn *b = n;
    return SColumnSort ( item, b -> scol );
}

int CC VColumnSort ( const void *item, const void *n )
{
    const VColumn *a = item;
    const VColumn *b = n;
    return SColumnSort ( a -> scol, b -> scol );
}


/* IdRange
 *  returns id range for column or page
 */
rc_t VColumnIdRangeRead ( const VColumn *self, int64_t *first, int64_t *last )
{
    rc_t rc;

    assert ( self != NULL );
    assert ( first != NULL && last != NULL );

    if ( self -> in == NULL )
        rc = RC ( rcVDB, rcColumn, rcAccessing, rcRange, rcNotOpen );
    else
    {
        /* a little silly, but set max range in 64-bit
           without complaints from 32-bit compilers */
        * first = 1;
        * first <<= 63;
        * last = ~ * first;

        /* now intersect this range with all physical sources */
        rc = VProductionColumnIdRange ( self -> in, first, last );
        if ( rc == 0 )
            return 0;
    }

    * first = * last = 0;

    return rc;
}

rc_t VColumnPageIdRange ( const VColumn *self, int64_t id, int64_t *first, int64_t *last )
{
    rc_t rc;

    assert ( self != NULL );
    assert ( first != NULL && last != NULL );

    if ( self -> in == NULL )
        rc = RC ( rcVDB, rcColumn, rcAccessing, rcRange, rcNotOpen );
    else
    {
        /* try to find page and get its range */
        rc = VProductionPageIdRange ( self -> in, id, first, last );
        if ( rc == 0 )
            return 0;
    }

    * first = * last = 0;

    return rc;
}


/* Datatype
 *  returns typedecl and/or typedef for column data
 *
 *  "type" [ OUT, NULL OKAY ] - returns the column type declaration
 *
 *  "def" [ OUT, NULL OKAY ] - returns the definition of the type
 *  returned in "type_decl"
 *
 * NB - one of "type" and "def" must be non-NULL
 */
rc_t VColumnDatatype ( const VColumn *self, VTypedecl *type, VTypedesc *desc )
{
    assert ( self != NULL );
    assert ( type != NULL || desc != NULL );

    if ( type != NULL )
        * type = self -> td;

    if ( desc != NULL )
        * desc = self -> desc;

    return 0;
}

/* Init - PRIVATE
 */
rc_t VColumnInit ( VColumn *self, const VSchema *schema, const SColumn *scol )
{
    rc_t rc = VSchemaDescribeTypedecl ( schema, & self -> desc, & scol -> td );
    if ( rc == 0 )
    {
        self -> schema = VSchemaAttach ( schema );
        self -> scol = scol;
        self -> td = scol -> td;
        self -> read_only = scol -> read_only;
    }
    return rc;
}


/* Make - PRIVATE
 *  make a read column
 */
rc_t VColumnMake ( VColumn **colp, const VSchema *schema, const SColumn *scol )
{
    rc_t rc;
    VColumn *col;

    assert ( colp != NULL );
    assert ( schema != NULL );

    col = calloc ( 1, sizeof * col );
    if ( col == NULL )
        rc = RC ( rcVDB, rcColumn, rcConstructing, rcMemory, rcExhausted );
    else
    {
        rc = VColumnInit ( col, schema, scol );
        if ( rc == 0 )
        {
            col -> read_only = true;
            * colp = col;
            return 0;
        }

        free ( col );
    }

    * colp = NULL;
    return rc;
}

rc_t VColumnReadCachedBlob ( const VColumn *self, const VBlob *vblob, int64_t row_id,
    uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len, uint32_t *repeat_count )
{
    uint64_t start;

    * elem_bits = VTypedescSizeof ( & self -> desc );
    * row_len = PageMapGetIdxRowInfo ( vblob -> pm, ( uint32_t ) ( row_id - vblob -> start_id ), boff, repeat_count );
    start = ( uint64_t ) boff [ 0 ] * elem_bits [ 0 ];
    * base = ( uint8_t* ) vblob -> data . base + ( start >> 3 );
    * boff = ( uint32_t ) start & 7;

    return 0;
}

rc_t VColumnReadBlob ( const VColumn *cself, const VBlob **vblobp, int64_t row_id,
   uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len, uint32_t * repeat_count, VBlobMRUCacheCursorContext *cctx )
{
    rc_t rc;

    if ( cself -> in == NULL )
        rc = RC ( rcVDB, rcColumn, rcReading, rcColumn, rcNotOpen );
    else
    {
        VBlob *vblob;
        rc = VProductionReadBlob ( cself -> in, & vblob, & row_id, 1, cctx );
        if ( rc == 0 )
        {
            VColumn *self = ( VColumn* ) cself;
            VColumnReadCachedBlob ( self, vblob, row_id, elem_bits, base, boff, row_len, repeat_count );

#if USE_KURT
            TRACK_BLOB ( VBlobRelease, self -> cache );
            ( void ) VBlobRelease ( self -> cache );

            if ( VBlobAddRef ( vblob ) != 0 )
                self -> cache = NULL;
            else
                self -> cache = vblob;
#endif

            * vblobp = vblob;
        }
    }

    return rc;
}

rc_t VColumnRead ( const VColumn *cself, int64_t row_id,
    uint32_t *elem_bits, const void **base, uint32_t *boff, uint32_t *row_len,
    VBlob ** vblob)
{
    rc_t rc;

    VBlob * dummy;
    if ( vblob == NULL )
    {
        dummy = NULL;
        vblob = & dummy;
    }

    if ( cself -> in == NULL )
        rc = RC ( rcVDB, rcColumn, rcReading, rcColumn, rcNotOpen );
    else
    {
        rc = VProductionReadBlob ( cself -> in, vblob, & row_id, 1, NULL );
        if ( rc == 0 )
        {
            VColumn *self = ( VColumn* ) cself;
            VColumnReadCachedBlob ( self, *vblob, row_id, elem_bits, base, boff, row_len, NULL );

#if USE_KURT
            TRACK_BLOB ( VBlobRelease, self -> cache );
            ( void ) VBlobRelease ( self -> cache );
            self -> cache = *vblob;
#else
            TRACK_BLOB ( VBlobRelease, *vblob );
            ( void ) VBlobRelease ( *vblob );
            *vblob = NULL;
#endif
        }
    }

    return rc;
}

/* IsStatic
 *  answers question: "does this column have the same value for every cell?"
 */
rc_t VColumnIsStatic ( const VColumn *self, bool *is_static )
{
    rc_t rc;

    if ( is_static == NULL )
        rc = RC ( rcVDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        * is_static = false;

        if ( self == NULL )
            rc = RC ( rcVDB, rcColumn, rcAccessing, rcSelf, rcNull );
        else if ( self -> in == NULL )
            rc = RC ( rcVDB, rcColumn, rcAccessing, rcColumn, rcNotOpen );
        else
        {
            return VProductionIsStatic ( self -> in, is_static );
        }
    }

    return rc;
}

/* GetKColumn
 *  drills down to physical production to get a KColumn,
 *  and if that fails, indicate whether the column is static
 */
rc_t VColumnGetKColumn ( const VColumn * self, struct KColumn ** kcol, bool * is_static )
{
    rc_t rc;

    bool dummy = false;

    if ( is_static == NULL )
        is_static = & dummy;

    if ( kcol == NULL )
        rc = RC ( rcVDB, rcColumn, rcAccessing, rcParam, rcNull );
    else
    {
        * kcol = NULL;
        * is_static = false;

        if ( self == NULL )
            rc = RC ( rcVDB, rcColumn, rcAccessing, rcSelf, rcNull );
        else if ( self -> in == NULL )
            rc = RC ( rcVDB, rcColumn, rcAccessing, rcColumn, rcNotOpen );
        else
        {
            return VProductionGetKColumn ( self -> in, kcol, is_static );
        }
    }

    return rc;
}
