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
#include "cursor-priv.h"
#include "prod-priv.h"
#undef KONST

#include <vdb/cursor.h>
#include <kdb/column.h>
#include <kdb/meta.h>
#include <klib/symbol.h>
#include <klib/debug.h>
#include <klib/log.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


#define ALLOW_V1_UPDATE 0

/*--------------------------------------------------------------------------
 * VPhysical
 */

/* LoadSchema
 *  looks in metadata for stored schema
 */
static
rc_t CC VPhysicalLoadV1Schema ( VPhysical *self,
    VTypedecl *td, VSchema *schema, const KMDataNode *node )
{
    rc_t rc;

    KMDataNodeSchemaFillData pb;
    pb . node = node;
    pb . pos = 0;
    pb . add_v0 = true;

    /* add stored declaration to cursor schema */
    rc = VSchemaParseTextCallback ( schema,
        "VPhysicalLoadV1Schema", KMDataNodeFillSchema, & pb );
   if ( rc == 0 )
    {
        size_t size;
        char type_expr [ 256 ];

        /* retrieve and resolve "type" attribute */
        rc = KMDataNodeReadAttr ( node, "type",
            type_expr, sizeof type_expr, & size );
        if ( rc == 0 )
            rc = VSchemaResolveTypedecl ( schema, td, "%s", type_expr );

        /* if a decoding schema exists */
        if ( rc == 0 && pb . pos != 0 )
        {
            char sphysical_name [ 512 ];

            /* preserve schema function expression */
            size_t decoding_xsize;
            char decoding_expr [ 256 ];
            rc = KMDataNodeReadAttr ( node, "schema",
                decoding_expr, sizeof decoding_expr, & decoding_xsize );
            if ( rc == 0 )
            {
                /* look for "encoding" */
                const KMDataNode *enc;
                rc = KMetadataOpenNodeRead ( self -> meta, & enc, "encoding" );
                if ( rc == 0 )
                {
#if ALLOW_V1_UPDATE
                    if ( ! self -> read_only )
                    {
                        /* add stored declaration to cursor schema */
                        pb . node = enc;
                        pb . pos = 0;
                        pb . add_v0 = true;
                        rc = VSchemaParseTextCallback ( schema,
                            "VPhysicalLoadV1Schema", KMDataNodeFillSchema, & pb );
                    }
                    if ( rc == 0 )
#endif
                    {
                        /* preserve schema function expression */
                        size_t encoding_xsize;
                        char encoding_expr [ 256 ], enc_type [ 256 ];
                        rc = KMDataNodeReadAttr ( enc, "schema",
                            encoding_expr, sizeof encoding_expr, & encoding_xsize );
                        if ( rc == 0 )
                        {
                            rc = KMDataNodeReadAttr ( enc, "type",
                                enc_type, sizeof enc_type, & size );
                        }
                        if ( rc == 0 )
                        {
#if ALLOW_V1_UPDATE
                            if ( self -> read_only )
                            {
#endif
                                /* build sphysical name */
                                sprintf ( sphysical_name, "%s_only", decoding_expr );

                                /* build physical decl */
                                pb . pos = sprintf ( pb . buff, "version 1;"
                                                     "physical %s %s:phys#1"
                                                     "{decode{%s k=@;return %s(k);}}"
                                                     , type_expr
                                                     , sphysical_name
                                                     , enc_type
                                                     , decoding_expr
                                    );
#if ALLOW_V1_UPDATE
                            }
                            else
                            {
                                /* strip off common namespace */
                                size_t i, ns_size;
                                string_match ( decoding_expr, decoding_xsize,
                                    encoding_expr, encoding_xsize, -1, & ns_size );
                                if ( ns_size != 0 )
                                {
                                    char *p = string_rchr ( decoding_expr, ns_size, ':' );
                                    ns_size = ( p == NULL ) ? 0U : ( uint32_t ) ( p - decoding_expr ) + 1U;
                                }

                                /* build sphysical name */
                                sprintf ( sphysical_name, "%s_%s", decoding_expr, & encoding_expr [ ns_size ] );
                                for ( i = ns_size; sphysical_name [ i ] != 0; ++ i )
                                {
                                    if ( sphysical_name [ i ] == ':' )
                                        sphysical_name [ i ] = '_';
                                }

                                /* build physical decl */
                                pb . pos = sprintf ( pb . buff, "version 1;"
                                                     "physical %s %s:phys#1"
                                                     "{encode{return %s(@);}"
                                                     "decode{%s k=@;return %s(k);}}"
                                                     , type_expr
                                                     , sphysical_name
                                                     , encoding_expr
                                                     , enc_type
                                                     , decoding_expr
                                    );
                            }
#endif
                        }
                    }

                    KMDataNodeRelease ( enc );
                }
                else if ( GetRCState ( rc ) == rcNotFound )
                {
                    /* build sphysical name */
                    sprintf ( sphysical_name, "%s_only", decoding_expr );

                    /* build decode-only physical decl */
                    pb . pos = sprintf ( pb . buff, "version 1;"
                                         "physical %s %s:phys#1"
                                         "{decode{opaque k=@;return %s(k);}}"
                                         , type_expr
                                         , sphysical_name
                                         , decoding_expr
                        );
                    rc = 0;
                }
                if ( rc == 0 )
                {
                    /* parse synthesized schema into cursor VSchema */
                    rc = VSchemaParseText ( schema,
                        "VPhysicalLoadV1Schema", pb . buff, pb . pos );
                    if ( rc == 0 )
                    {
                        VTypedecl etd;

                        /* create a new expression object */
                        sprintf ( pb . buff, "%s:phys#1", sphysical_name );
                        rc = VSchemaImplicitPhysEncExpr ( schema, & etd,
                            & self -> enc, pb . buff, "VPhysicalLoadV1Schema" );
                        if ( rc != 0 )
                        {
                            PLOGERR ( klogInt, ( klogInt, rc, "failed to establish column type from '$(expr)'",
                                       "expr=%s", pb . buff ));
                        }
                        else if ( self -> smbr != NULL && self -> smbr -> type == NULL )
                        {
                            /* back-patch schema */
                            ( ( SPhysMember* ) self -> smbr ) -> type = self -> enc;
                            atomic32_inc ( & ( ( SExpression* ) self -> enc ) -> refcount );
                        }
                    }
                }
            }
        }
    }

    KMDataNodeRelease ( node );
    return rc;
}

static
rc_t CC VPhysicalLoadSchema ( VPhysical *self,
    VTypedecl *td, VSchema *schema, const KMDataNode *node )
{
    rc_t rc;

    KMDataNodeSchemaFillData pb;
    pb . node = node;
    pb . pos = 0;
    pb . add_v0 = false;

    /* add stored declaration to cursor schema */
    rc = VSchemaParseTextCallback ( schema,
        "VPhysicalLoadSchema", KMDataNodeFillSchema, & pb );
    if ( rc == 0 )
    {
        /* retrieve fully-resolved type attribute */
        rc = KMDataNodeReadAttr ( node, "type",
            pb . buff, sizeof pb . buff, & pb . pos );
        if ( rc == 0 )
            rc = VSchemaResolveTypedecl ( schema, td, "%s", pb . buff );
        if ( rc == 0 )
        {
            /* get encoding expression */
            rc = KMDataNodeReadAttr ( node, "expr",
                pb . buff, sizeof pb . buff, & pb . pos );
            if ( rc == 0 )
            {
                VTypedecl etd;

                /* create a new expression object */
                rc = VSchemaImplicitPhysEncExpr ( schema, & etd,
                    & self -> enc, pb . buff, "VPhysicalLoadSchema" );
                if ( rc != 0 )
                {
                    PLOGERR ( klogInt, ( klogInt, rc, "failed to establish column type from '$(expr)'",
                               "expr=%s", pb . buff ));
                }

                /* match SPhysical type against stated type */
                else if ( ! VTypedeclToTypedecl ( & etd, schema, td, NULL, NULL ) )
                {
                    rc = RC ( rcVDB, rcColumn, rcLoading, rcType, rcInconsistent );
                    PLOGERR ( klogInt, ( klogInt, rc, "inconsistent metadata for column '$(name)'"
                               , "name=%.*s"
                               , ( int ) self -> smbr -> name -> name . size
                               , self -> smbr -> name -> name . addr ));
                }
            }
            else if ( GetRCState ( rc ) == rcNotFound )
            {
                rc = 0;
            }
        }
    }

    KMDataNodeRelease ( node );
    return rc;
}

rc_t VPhysicalLoadMetadata ( VPhysical *self, VTypedecl *td, VSchema *schema )
{
    /* capture fixed row length */
    const KMDataNode *node;
    rc_t rc = KMetadataOpenNodeRead ( self -> meta, & node, "row-len" );
    if ( rc == 0 )
    {
        rc = KMDataNodeReadAsU32 ( node, & self -> fixed_len );
        KMDataNodeRelease ( node );
        if ( rc != 0 )
            return rc;
    }

    /* look for "schema" root node */
    rc = KMetadataOpenNodeRead ( self -> meta, & node, "schema" );
    if ( rc == 0 )
        return VPhysicalLoadSchema ( self, td, schema, node );
    if ( GetRCState ( rc ) != rcNotFound )
        return rc;

    /* appears to be an older column */
    self -> v01 = true;

    /* look for "decoding" root node */
    rc = KMetadataOpenNodeRead ( self -> meta, & node, "decoding" );
    if ( rc == 0 )
        return VPhysicalLoadV1Schema ( self, td, schema, node );
    if ( GetRCState ( rc ) != rcNotFound )
        return rc;

    /* benign error for very old columns */
    rc = RC ( rcVDB, rcColumn, rcLoading, rcSchema, rcNotFound );

    DBGMSG(DBG_VDB, DBG_FLAG(DBG_VDB_VDB),
        ("VPhysicalLoadMetadata = %d\n", rc));

    return rc;
}
