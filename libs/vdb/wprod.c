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

#define TRACK_REFERENCES 0

#define KONST const
#include "prod-priv.h"
#include "prod-expr.h"
#include "schema-priv.h"
#include "schema-expr.h"
#include "cursor-priv.h"
#include "column-priv.h"
#include "phys-priv.h"
#undef KONST

#include <vdb/cursor.h>
#include <klib/symbol.h>
#include <klib/log.h>
#include <klib/debug.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>



/*--------------------------------------------------------------------------
 * VProdResolve
 */

/* ResolveColumn
 *  resolves a column using read/write expression
 */
rc_t VProdResolveColumnRoot ( const VProdResolve *self,
    VProduction **out, const SColumn *scol )
{
    rc_t rc;
    WColumn *wcol;
    VTypedesc desc;
    const char *name;
    VCursor *curs = self -> curs;
    VProduction * in;

    * out = NULL;

    /* decide upon behavior */
    if ( VCursorIsReadOnly ( curs ) )
    {
        rc = VProdResolveColumnRead ( self, out, scol );
        if ( rc == 0 && * out <= FAILED_PRODUCTION )
            return RC ( rcVDB, rcCursor, rcOpening, rcColumn, rcUndefined );
        return rc;
    }

    /* write-only cursor must have existing column */
    wcol = VCursorCacheGet ( VCursorColumns ( curs ), & scol -> cid );
    if ( wcol == NULL )
        return 0;

    /* not intended to be reentrant */
    assert ( wcol -> val == NULL );

    /* evaluate input expression */
    if ( scol -> validate == NULL )
    {
        /* use normal read expression */
        rc = VProdResolveColumnRead ( self, &in, scol );
    }
    else
    {
        VFormatdecl fd;

        /* create fmtdecl from typedecl */
        memset ( & fd, 0, sizeof fd );

        VDB_DEBUG ( ( "resolving column '%N' validate expression.\n", scol -> name ) );

        /* use validation expression */
        rc = VProdResolveExpr ( self, &in, & desc, & fd, scol -> validate, false );
    }

    /* check failures */
    if ( rc != 0 )
    {
        VDB_DEBUG ( ( "failed to resolve column '%N' - %R.\n", scol -> name, rc ) );
        return rc;
    }
    if ( in <= FAILED_PRODUCTION )
    {
        VDB_DEBUG ( ( "failed to resolve column '%N' - NULL or failed production.\n", scol -> name ) );
        return RC ( rcVDB, rcCursor, rcOpening, rcColumn, rcUndefined );
    }

    /* column name */
    name = scol -> name -> name . addr;

    /* pick up production */
    if ( scol -> validate != NULL )
    {
        rc = VSimpleProdMake ( & wcol -> val, self -> owned, self -> curs,
            prodSimpleCast, name, NULL, NULL, NULL, in, chainDecoding );
        if ( rc != 0 )
            return rc;
    }

    /* create implicit comparison function */
    else
    {
        /* need an output production */
        if ( wcol -> out == NULL )
        {
            rc = VColumnProdMake ( & wcol -> out, self -> owned,
                & wcol -> dad, prodColumnOut, name );
            if ( rc != 0 )
                return rc;
        }

        /* create comparison func */
        rc = VFunctionProdMakeBuiltInComp ( & wcol -> val, self -> owned,
            name, self, wcol -> out, in );
        if ( rc != 0 )
            return rc;
    }

    /* install trigger */
    rc = VCursorInstallTrigger ( curs, wcol -> val );
    if ( rc == 0 )
        * out = wcol -> val;

    return rc;
}

rc_t VProdResolveColumn ( const VProdResolve *self,
    VProduction **out, const SColumn *scol, bool alt )
{
    rc_t rc;
    VColumn *vcol;
    WColumn *wcol;
    VCursor *curs = self -> curs;

    /* decide upon behavior */
    if ( VCursorIsReadOnly ( curs ) )
    {
        if ( alt )
        {
            /* TODO: Generate warning message */
            return RC ( rcVDB, rcCursor, rcOpening, rcSchema, rcInvalid );
        }
        vcol = VCursorCacheGet ( VCursorColumns ( curs ), & scol -> cid );
        if ( vcol == NULL )
        {
            rc = VCursorMakeColumn ( curs, & vcol, scol, self -> cx_bind );
            if ( rc != 0 )
                return rc;

#if OPEN_COLUMN_ALTERS_ROW
            rc = VectorAppend ( VCursorGetRow ( curs ), & vcol -> ord, vcol );
            if ( rc != 0 )
            {
                VColumnWhack ( vcol, NULL );
                return rc;
            }
#endif
            rc = VCursorCacheSet ( VCursorColumns ( curs ), & scol -> cid, vcol );
            if ( rc != 0 )
            {
#if OPEN_COLUMN_ALTERS_ROW
                void *ignore;
                VectorSwap ( VCursorGetRow ( curs ), vcol -> ord, NULL, & ignore );
                vcol -> ord = 0;
#endif
                VColumnWhack ( vcol, NULL );
                return rc;
            }
        }

        return VProdResolveColumnRead ( self, out, scol );
    }

    /* write cursor but read side */
    if ( self -> chain == chainDecoding )
    {
        if ( alt )
        {
            /* TODO: Generate warning message */
            return RC ( rcVDB, rcCursor, rcOpening, rcSchema, rcInvalid );
        }

        return VProdResolveColumnRead ( self, out, scol );
    }

    /* get existing column */
    wcol = VCursorCacheGet ( VCursorColumns ( curs ), & scol -> cid );
    if ( wcol == NULL )
    {
        /* normally write-only cursor must have existing column */
        if ( ! self -> discover_writable_columns )
            return 0;

        /* auto-create writable column for purposes of discovery */
        if ( scol -> read_only )
            return 0;
        rc = VCursorMakeColumn ( curs, & vcol, scol, self -> cx_bind );
        if ( rc != 0 )
            return rc;

        /* add it to the row as if user had done it */
        rc = VectorAppend ( VCursorGetRow ( curs ), & vcol -> ord, vcol );
        if ( rc == 0 )
        {
            /* add it to the indexed vector */
            rc = VCursorCacheSet ( VCursorColumns ( curs ), & scol -> cid, vcol );
            if ( rc != 0 )
            {
                void *ignore;
                VectorSwap ( VCursorGetRow ( curs ), vcol -> ord, NULL, & ignore );
                vcol -> ord = 0;
            }
        }

        if ( rc != 0 )
        {
            VColumnWhack ( vcol, NULL );
            return rc;
        }

        wcol = ( WColumn* ) vcol;
    }

    /* create output production as required */
    if ( wcol -> out == NULL )
    {
        const char *name = scol -> name -> name . addr;
        rc = VColumnProdMake ( & wcol -> out, self -> owned,
            & wcol -> dad, prodColumnOut, name );
        if ( rc != 0 )
            return rc;
    }
    if ( alt )
    {
        * out = wcol -> dad . in;
        assert ( * out != NULL );
    }
    else
    {
        * out = wcol -> out;
    }
    return 0;
}

/* ResolvePhysical
 *  resolves a physical column
 */
static
rc_t VProdResolvePhysicalWrite ( const VProdResolve *self, VPhysical *phys )
{
    VTypedesc desc;
    VFormatdecl fd;
    VProdResolve pr;
    VProduction *prod;
    VCursor *curs = self -> curs;

    const char *name;
    const SExpression *enc;
    const SPhysMember *smbr;

    /* open the physical column for write
       load column metadata/schema, complete
       physical member description. */
    rc_t rc = VPhysicalOpenWrite ( phys,
        ( VSchema* ) self -> schema, VCursorGetTable ( curs ) );
    if ( rc != 0 )
        return rc;

    /* there are two conditions under which a physical member
       definition would be incommplete prior to opening the
       column: 1) if the physical column were only forwarded
       within table schema, or 2) if the column were added as
       the result of a file system scan.

       for the column to be writable, it must have had a complete
       member definition from table schema, with a type and an
       assignment expression, or it must have been added as the
       result of an fs scan with a simple reciprocal expression.
       the test for these two cases is for a resolved typedecl
       and an assignment expression. */

    /* nothing more to do if column does not exist
       and member was undeclared, or is declared read-only */
    smbr = phys -> smbr;
    if ( smbr -> td . type_id == 0 || smbr -> expr == NULL )
        return 0;

    /* build fmtdecl */
    fd . td = smbr -> td;
    fd . fmt = 0;

    /* shift to encode chain */
    pr = * self;
    pr . chain = chainEncoding;

    /* resolve the input expression */
    rc = VProdResolveExpr ( & pr, & phys -> in, & desc, & fd, smbr -> expr, false );
    if ( rc == 0 && phys -> in == NULL )
        return RC ( rcVDB, rcCursor, rcOpening, rcColumn, rcUndefined );

    /* NB - at this point, fd and desc
       represent the column's well-defined type */

    /* member name */
    name = smbr -> name -> name . addr;

    /* physical encoding */
    enc = phys -> enc;
    if ( enc == NULL )
        enc = smbr -> type;

    /* build encoding schema in steps:
         in <- page-to-blob
    */
    rc = VSimpleProdMake ( & prod, pr . owned,  pr . curs,
        prodSimplePage2Blob, name, & fd, & desc, NULL, phys -> in, chainEncoding );
    if ( rc == 0 && enc != NULL )
    {
        /* in <- p2b <- encoding-func */
        pr . blobbing = true;
        rc = VProdResolveEncodingExpr ( & pr, & prod,
            prod, ( const SPhysEncExpr* ) enc );
        if ( rc == 0 )
        {
            fd = prod -> fd;
            desc = prod -> desc;
        }
    }
    if ( rc == 0 )
    {
        rc = VSimpleProdMake ( & phys -> b2s, pr . owned, pr . curs,
            prodSimpleBlob2Serial, name, & fd, & desc, NULL, prod, chainEncoding );
    }

    return rc;
}

rc_t VProdResolvePhysical ( const VProdResolve *self, VPhysical *phys )
{
    /* build encoding chain if writable cursor */
    if ( ! VCursorIsReadOnly ( self -> curs ) )
    {
        rc_t rc = VProdResolvePhysicalWrite ( self, phys );
        if ( rc != 0 || self -> discover_writable_columns )
            return rc;
    }

    /* build decoding chain */
    return VProdResolvePhysicalRead ( self, phys );
}

/*--------------------------------------------------------------------------
 * VColumnProd
 *  message redirect to VColumn
 */

rc_t VColumnProdMake ( VProduction **prodp, Vector *owned,
    VColumn *col, int sub, const char *name )
{
    const SColumn *scol = col -> scol;

    /* why was this changed to get the column td from SColumn? */
    VTypedesc desc;
    rc_t rc = VSchemaDescribeTypedecl ( col -> schema,
        & desc, & col -> scol -> td );
    if ( rc != 0 )
        * prodp = NULL;
    else
    {
        VColumnProd *prod;

        /* construct an fd because column does not have one */
        VFormatdecl fd;
        fd . td = scol -> td;
        fd . fmt = 0;

        rc = VProductionMake ( prodp, owned, sizeof * prod,
            prodColumn, sub, name, & fd, & desc, NULL, chainEncoding );
        if ( rc == 0 )
        {
            prod = ( VColumnProd* ) * prodp;
            prod -> col = col;
        }
    }
    return rc;
}

void VColumnProdDestroy ( VColumnProd *self )
{
}


/* Read
 */
rc_t VColumnProdRead ( VColumnProd *self, struct VBlob **vblob, int64_t id )
{
    WColumn *wcol = ( WColumn* ) self -> col;

    switch ( self -> dad . sub )
    {
    case prodColumnOut:
        return WColumnReadBlob ( wcol, vblob, id );
    }

    return RC ( rcVDB, rcColumn, rcReading, rcProduction, rcCorrupt );
}
