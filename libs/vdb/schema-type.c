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

#include "schema-priv.h"
#include "schema-parse.h"
#include "schema-expr.h"
#include "schema-dump.h"
#include "dbmgr-priv.h"

#include <sra/types.h>
#include <klib/symbol.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


/*--------------------------------------------------------------------------
 * SFormat
 */

/* Find
 */
SFormat *VSchemaFindFmtid ( const VSchema *self, uint32_t id )
{
    SFormat *fmt = VectorGet ( & self -> fmt, id );
    while ( fmt == NULL )
    {
        self = self -> dad;
        if ( self == NULL )
            break;
        fmt = VectorGet ( & self -> fmt, id );
    }
    return fmt;
}

/* Mark
 */
void CC SFormatClearMark ( void *item, void *ignore )
{
    SFormat *self = item;
    self -> marked = false;
}

void SFormatMark ( const SFormat *cself )
{
    SFormat *self = ( SFormat* ) cself;
    if ( cself != NULL && ! cself -> marked )
    {
        self -> marked = true;
        SFormatMark ( self -> super );
    }
}


/* Dump
 *  dump "fmtdef", dump object
 */
rc_t SFormatDump ( const SFormat *self, SDumper *b )
{
    return FQNDump ( self != NULL ? self -> name : NULL, b );
}

bool CC SFormatDefDump ( void *item, void *data )
{
    SDumper *b = data;
    const SFormat *self = ( const void* ) item;

    if ( SDumperMarkedMode ( b ) && ! self -> marked )
        return false;

    if ( SDumperMode ( b ) == sdmCompact )
    {
        b -> rc = ( self -> super != NULL ) ?
            SDumperPrint ( b, "fmtdef %N %N;", self -> super -> name, self -> name ):
            SDumperPrint ( b, "fmtdef %N;", self -> name );
    }
    else
    {
        b -> rc = ( self -> super != NULL ) ?
            SDumperPrint ( b, "fmtdef %N %N;\n", self -> super -> name, self -> name ):
            SDumperPrint ( b, "fmtdef %N;\n", self -> name );
    }

    if ( b -> rc == 0 )
        b -> rc = AliasDump ( self -> name, b );

    return ( b -> rc != 0 ) ? true : false;
}


/*--------------------------------------------------------------------------
 * SDatatype
 */

/* IntrinsicDim
 */
#if SLVL >= 1
uint32_t SDatatypeIntrinsicDim ( const SDatatype *self )
{
    uint32_t dim = 1;
    while ( 1 )
    {
        /* if type has no supertype or supertype is opaque
           then this is the base intrinsic type */
        const SDatatype *super = self -> super;
        if ( super == NULL || super -> domain == 0 )
            break;

        /* accumulate the vector dimension of this type */
        dim *= self -> dim;
        self = super;
    }

    return dim;
}

/* Find
 */
SDatatype *VSchemaFindTypeid ( const VSchema *self, uint32_t id )
{
    SDatatype *dt = VectorGet ( & self -> dt, id );
    while ( dt == NULL )
    {
        self = self -> dad;
        if ( self == NULL )
            break;
        dt = VectorGet ( & self -> dt, id );
    }
    return dt;
}


/* DescribeTypedecl
 *  produce a description of typedecl properties
 */
LIB_EXPORT rc_t CC VSchemaDescribeTypedecl ( const VSchema *self,
    VTypedesc *desc, const VTypedecl *td )
{
    rc_t rc;
    if ( desc == NULL )
        rc = RC ( rcVDB, rcSchema, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcSchema, rcAccessing, rcSelf, rcNull );
        else if ( td == NULL )
            rc = RC ( rcVDB, rcSchema, rcAccessing, rcParam, rcNull );
        else
        {
            const SDatatype *dt = VSchemaFindTypeid ( self, td -> type_id );
            if ( dt == NULL )
                rc = RC ( rcVDB, rcSchema, rcAccessing, rcType, rcNotFound );
            else
            {
                /* initial dimension from typedecl itself */
                desc -> intrinsic_dim = td -> dim ? td -> dim : 1;

                /* domain comes from the original type */
                desc -> domain = dt -> domain;

                while ( 1 )
                {
                    const SDatatype *super = dt -> super;
                    if ( super == NULL || super -> domain == 0 )
                        break;

                    /* become supertype */
                    assert ( super != dt );
                    desc -> intrinsic_dim *= dt -> dim;
                    dt = super;
                }

                /* take size from intrinsic */
                desc -> intrinsic_bits = dt -> size;
                return 0;
            }
        }

        memset ( desc, 0, sizeof * desc );
    }
    return rc;
}


/* DescribeTypedef - PRIVATE
 *  a type is defined as either:
 *
 *     'typedef' <type> ';'
 *  or
 *     'typedef' <supertype> <type> '[' <dim> ']' ';'
 */
LIB_EXPORT rc_t CC VSchemaDescribeTypedef ( const VSchema *self,
    VTypedef *def, uint32_t type_id )
{
    rc_t rc;
    if ( def == NULL )
        rc = RC ( rcVDB, rcSchema, rcAccessing, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcSchema, rcAccessing, rcSelf, rcNull );
        else
        {
            const SDatatype *dt = VSchemaFindTypeid ( self, type_id );
            if ( dt == NULL )
                rc = RC ( rcVDB, rcSchema, rcAccessing, rcType, rcNotFound );
            else
            {
                /* id of <type> */
                def -> type_id = dt -> id;

                /* check for supertype */
                if ( dt -> super == NULL )
                {
                    /* null id */
                    def -> super_id = 0;
                    /* sizeof ( <type> ) in bits */
                    def -> type_size = dt -> size;
                    assert ( dt -> dim <= 1 );
                }
                else
                {
                    /* id of <supertype> */
                    def -> super_id = dt -> super -> id;
                    /* sizeof ( <super-type> ) in bits */
                    def -> type_size = dt -> super -> size;
                }

                /* 'dim' from 'typedef <supertype> <type> [ dim ];' */
                def -> dim = dt -> dim;

                /* the only bit of domain information is whether the type is signed */
                def -> sign = 0;
                switch ( dt -> domain )
                {
                case vtdInt:
                case vtdFloat:
                    def -> sign = 1;
                    break;
                }

                return 0;
            }
        }

        memset ( def, 0, sizeof * def );
    }

    return rc;
}

/* Mark
 */
void CC SDatatypeClearMark ( void *item, void *ignore )
{
    SDatatype *self = item;
    self -> marked = false;
}

void SDatatypeMark ( const SDatatype *cself )
{
    SDatatype *self = ( SDatatype* ) cself;
    if ( cself != NULL && ! cself -> marked )
    {
        self -> marked = true;
        SDatatypeMark ( self -> super );
    }
}

/* Dump
 */
rc_t SDatatypeDump ( const SDatatype *self, SDumper *d )
{
    return FQNDump ( self != NULL ? self -> name : NULL, d );
}

bool CC SDatatypeDefDump ( void *item, void *data )
{
    SDumper *b = data;
    const char *dimfmt;
    const SDatatype *self = ( const void* ) item;

    if ( SDumperMarkedMode ( b ) && ! self -> marked )
        return false;

    switch ( SDumperMode ( b ) )
    {
    case sdmCompact:
        dimfmt = "[%u]"; break;
    default:
        dimfmt = " [ %u ]";
    }

    b -> rc = SDumperPrint ( b, "typedef %N %N",
        self -> super -> name, self -> name );
    if ( b -> rc == 0 && self -> dim > 1 )
        b -> rc = SDumperPrint ( b, dimfmt, self -> dim );
    if ( b -> rc == 0 )
    {
        if ( SDumperMode ( b ) == sdmCompact )
            b -> rc = SDumperWrite ( b, ";", 1 );
        else
        {
#if _DEBUGGING && 1
            b -> rc = SDumperPrint ( b, "; /* size %u */\n", self -> size );
#else
            b -> rc = SDumperWrite ( b, ";\n", 2 );
#endif
        }
    }

    if ( b -> rc == 0 )
        b -> rc = AliasDump ( self -> name, b );

    return ( b -> rc != 0 ) ? true : false;
}
#endif

/*--------------------------------------------------------------------------
 * STypesetMbr
 *  a typedecl that can be tested for uniqueness
 */
typedef struct STypesetMbr STypesetMbr;
struct STypesetMbr
{
    BSTNode n;
    VTypedecl td;
};

/* Whack
 */
#define STypesetMbrWhack BSTreeMbrWhack

/* Cmp
 * Sort
 */
static
int64_t VTypedeclCmp ( const VTypedecl *a, const VTypedecl *b )
{
    if ( a -> type_id != b -> type_id )
        return ( int64_t ) a -> type_id - ( int64_t ) b -> type_id;
    return ( int64_t ) a -> dim - ( int64_t ) b -> dim;
}

static
int64_t CC STypesetMbrSort ( const BSTNode *item, const BSTNode *n )
{
    const STypesetMbr *a = ( const STypesetMbr* ) item;
    const STypesetMbr *b = ( const STypesetMbr* ) n;
    return VTypedeclCmp ( & a -> td, & b -> td );
}


/*--------------------------------------------------------------------------
 * VTypedecl
 */

#if SLVL >= 1

/* ToText
 *  convert a VTypedecl into canonical text
 *
 *  "buffer" [ OUT ] and "bsize" [ IN ] - output buffer for
 *  NUL terminated type declaration string
 *
 *  "fmtdecl" [ IN ] - binary representation of fmtdecl
 */
LIB_EXPORT rc_t CC VTypedeclToText ( const VTypedecl *self,
    const VSchema *schema, char *buffer, size_t bsize )
{
    rc_t rc;

    if ( bsize == 0 )
        rc = RC ( rcVDB, rcType, rcConverting, rcBuffer, rcInsufficient );
    else if ( buffer == NULL )
        rc = RC ( rcVDB, rcType, rcConverting, rcBuffer, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcVDB, rcType, rcConverting, rcSelf, rcNull );
        else if ( schema == NULL )
            rc = RC ( rcVDB, rcType, rcConverting, rcSchema, rcNull );
        else
        {
            size_t num_writ;
            rc = VSchemaToText ( schema, buffer, bsize - 1, & num_writ, "%T", self );
            if ( rc == 0 )
            {
                buffer [ num_writ ] = 0;
                return 0;
            }
        }

        buffer [ 0 ] = 0;
    }

    return rc;
}


/* ToSupertype
 *  attempt to cast a typedecl to a size-equivalent supertype decl
 *
 *  "schema" [ IN ] - schema object that would know about this type
 *
 *  "cast" [ OUT ] - return parameter for supertype decl
 *  valid only if function returns true.
 *
 *  returns true if cast succeeded
 */
LIB_EXPORT bool CC VTypedeclToSupertype ( const VTypedecl *self,
    const VSchema *schema, VTypedecl *cast )
{
    if ( self != NULL && schema != NULL )
    {
        const SDatatype *dt = VSchemaFindTypeid ( schema, self -> type_id );
        if ( dt != NULL )
        {
            uint32_t dim = dt -> dim;
            dt = dt -> super;
            if ( dt != NULL && dt -> domain != 0 )
            {
                if ( cast != NULL )
                {
                    cast -> type_id = dt -> id;
                    cast -> dim = self -> dim * dim;
                }

                return true;
            }
        }
    }
    return false;
}


/* ToTypedecl
 *  attempt to cast a typedecl to a size-equivalent ancestor decl
 *
 *  "schema" [ IN ] - schema object that would know about this type
 *
 *  "ancestor" [ IN ] - target typedecl or typeset for cast
 *
 *  "cast" [ OUT, NULL ] - return parameter for ancestor decl
 *  valid only if function returns true. if "ancestor" was a typeset,
 *  the value of "cast" will be the closest matching ancestral type.
 *
 *  "distance" [ OUT, NULL OKAY ] - optional linear measure of casting
 *  generations. valid only if function returns true. a value of 0 means
 *  that "self" is a direct match with "ancestor", a value of 1 means that
 *  "self" is an immediate subtype of "ancestor", etc.
 *
 *  returns true if cast succeeded
 */
static
bool VTypedecl2Typedecl ( const VTypedecl *self,
    const VSchema *schema, const VTypedecl *ancestor,
    VTypedecl *cast, uint32_t *distance )
{
    const SDatatype *src;
    uint32_t i, type_id = self -> type_id;
    uint32_t gramps = ancestor -> type_id;
    uint32_t dim = self -> dim;

    /* casting from "any" */
    if ( type_id == 0 )
    {
        /* "any" to "any" isn't a cast... or is it? */
        if ( gramps == 0 )
            return false;

        /* produce output */
        if ( cast != NULL && ( const VTypedecl* ) cast != ancestor )
            * cast = * ancestor;

        /* record distance */
        if ( distance != NULL )
            * distance = 0;

        return true;
    }

    /* casting to "any" */
    if ( gramps == 0 )
    {
        /* produce output */
        if ( cast != NULL && ( const VTypedecl* ) cast != self )
            * cast = * self;

        /* record distance */
        if ( distance != NULL )
            * distance = 0;

        return true;
    }

    /* special case for identity */
    if ( type_id == gramps )
    {
        /* detect variable dimension */
        if ( dim == 0 )
        {
            dim = ancestor -> dim;
            if ( ancestor -> dim == 0 )
                return false;
        }

        /* if ancestor has variable dimension */
        if ( ancestor -> dim == 0 )
        {
            if ( cast != NULL )
            {
                cast -> type_id = type_id;
                cast -> dim = dim;
            }
        }
        else
        {
            /* must have identical fixed dimension */
            if ( dim != ancestor -> dim )
                return false;

            /* produce output */
            if ( cast != NULL && ( const VTypedecl* ) cast != ancestor )
                * cast = * ancestor;
        }

        /* record distance */
        if ( distance != NULL )
            * distance = 0;

        return true;
    }
    /***** Temporary fix for duplicate types****/
    {
	char bufa[100];
	char bufb[100];
	if(   self->dim == ancestor->dim
	   && VTypedeclToText(     self, schema, bufa, sizeof(bufa))== 0
           && VTypedeclToText( ancestor, schema, bufb, sizeof(bufb))== 0
           && strcmp(bufa,bufb) == 0){
		if ( cast != NULL && ( const VTypedecl* ) cast != ancestor ) * cast = * ancestor;
		if ( distance != NULL ) * distance = 0;
		return true;
        }
    }
	
    /************************/

    /* find type */
    src = VSchemaFindTypeid ( schema, type_id );
    if ( src == NULL )
        return false;

    /* cast toward gramps */
    for ( i = 0; src -> id > gramps; ++ i )
    {
        dim *= src -> dim;
        src = src -> super;
        if ( src == NULL || src -> domain == 0 )
            return false;
    }

    /* if not met */
    if ( src -> id != gramps )
        return false;

    /* handle variable dimension */
    if ( dim == 0 )
    {
        dim = ancestor -> dim;
        if ( ancestor -> dim == 0 )
            return false;
    }
    if ( ancestor -> dim == 0 )
    {
        if ( cast != NULL )
        {
            cast -> type_id = gramps;
            cast -> dim = dim;
        }
    }
    else
    {
        if ( dim != ancestor -> dim )
            return false;
        if ( cast != NULL && ( const VTypedecl* ) cast != ancestor )
            * cast = * ancestor;
    }

    /* produce output */
    if ( distance != NULL )
        * distance = i;
                            
    return true;
}

static
bool STypesetdecl2Typedecl ( const STypeset *self, uint32_t sdim,
    const VSchema *schema,  const VTypedecl *ancestor,
    VTypedecl *cast, uint32_t *distance )
{
    bool early;
    VTypedecl btd;
    uint32_t i, count, best;

    /* ambiguous if casting to "any" */
    if ( ancestor -> type_id == 0 )
        return false;

    /* can exit early if only testing castability */
    early = ( cast == NULL && distance == NULL ) ? true : false;

    /* walk set */
    for ( best = ~ 0U, count = self -> count, i = 0; i < count; ++ i )
    {
        uint32_t dist;

        /* convert set member to vector */
        VTypedecl td = self -> td [ i ];
        td . dim *= sdim;

        /* cast to ancestor */
        if ( VTypedecl2Typedecl ( & td, schema, ancestor, & td, & dist ) )
        {
            /* if only testing castability */
            if ( early )
                return true;

            /* if match is better */
            if ( dist < best )
            {
                if ( distance != NULL )
                    * distance = dist;
                if ( dist == 0 )
                {
                    if ( cast != NULL )
                        * cast = td;
                    return true;
                }

                btd = td;
                best = dist;
            }
        }
    }

    if ( ( int32_t ) best > 0 )
    {
        if ( cast != NULL )
            * cast = btd;
        return true;
    }

    return false;
}

static
bool VTypedecl2STypesetdecl ( const VTypedecl *self,
    const VSchema *schema, const STypeset *dst, uint32_t ddim,
    VTypedecl *cast, uint32_t *distance )
{
    const SDatatype *src;
    uint32_t i, j, count, type_id, sdim;

    /* ambiguous if casting from "any" */
    if ( self -> type_id == 0 )
        return false;

    /* find source type */
    src = VSchemaFindTypeid ( schema, self -> type_id );
    if ( src == NULL )
        return false;

    /* current dimension */
    sdim = self -> dim;

    /* perform cast */
    for ( count = dst -> count, type_id = src -> id, i = 0; ; type_id = src -> id, ++ i )
    {
        /* scan set for type match */
        for ( j = 0; j < count; ++ j )
        {
            if ( type_id == dst -> td [ j ] . type_id )
            {
                /* this is a hit on type */
                if ( sdim == dst -> td [ j ] . dim * ddim )
                {
                    /* best match is first match */
                    if ( distance != NULL )
                        * distance = i;
                    if ( cast != NULL )
                    {
                        cast -> type_id = type_id;
                        cast -> dim = sdim;
                    }
                    return true;
                }
                break;
            }
        }

        /* did not match any of them */
        sdim *= src -> dim;
        src = src -> super;
        if ( src == NULL || src -> domain == 0 )
            break;
    }

    return false;
}

static
bool VTypesetdeclToTypedecl ( const VTypedecl *self,
    const VSchema *schema,  const VTypedecl *ancestor,
    VTypedecl *cast, uint32_t *distance )
{
    /* find source typeset */
    const STypeset *src = VSchemaFindTypesetid ( schema, self -> type_id );

    /* not found or empty */
    if ( src == NULL || src -> count == 0 )
        return false;

    /* if has single type */
    if ( src -> count == 1 )
    {
        VTypedecl td = src -> td [ 0 ];
        td . dim *= self -> dim;
        return VTypedecl2Typedecl ( & td, schema, ancestor, cast, distance );
    }

    /* perform cast */
    return STypesetdecl2Typedecl ( src, self -> dim, schema, ancestor, cast, distance );
}

static
bool VTypedeclToTypesetdecl ( const VTypedecl *self,
    const VSchema *schema,  const VTypedecl *ancestor,
    VTypedecl *cast, uint32_t *distance )
{
    /* find ancestor typeset */
    const STypeset *dst = VSchemaFindTypesetid ( schema, ancestor -> type_id );

    /* if not found or empty */
    if ( dst == NULL || dst -> count == 0 )
        return false;

    /* if has a single type */
    if ( dst -> count == 1 )
    {
        VTypedecl td = dst -> td [ 0 ];
        td . dim *= ancestor -> dim;
        return VTypedecl2Typedecl ( self, schema, & td, cast, distance );
    }

    /* find best match */
    return VTypedecl2STypesetdecl ( self, schema, dst, ancestor -> dim, cast, distance );
}

static
bool VTypesetdeclToTypesetdecl ( const VTypedecl *self,
    const VSchema *schema,  const VTypedecl *ancestor,
    VTypedecl *cast, uint32_t *distance )
{
    bool early;
    VTypedecl btd, td;
    uint32_t i, count, best;
    const STypeset * src, *dst;

    /* find source typeset */
    src = VSchemaFindTypesetid ( schema, self -> type_id );

    /* not found or empty */
    if ( src == NULL || src -> count == 0 )
        return false;

    /* find ancestor typeset */
    dst = VSchemaFindTypesetid ( schema, ancestor -> type_id );

    /* if not found or empty */
    if ( dst == NULL || dst -> count == 0 )
        return false;

    /* if has a single type */
    if ( src -> count == 1 )
    {
        td = src -> td [ 0 ];
        td . dim *= self -> dim;
        if ( dst -> count == 1 )
        {
            btd = dst -> td [ 0 ];
            btd . dim *= ancestor -> dim;
            return VTypedecl2Typedecl ( & td, schema, & btd, cast, distance );
        }
        return VTypedecl2STypesetdecl ( & td, schema, dst, ancestor -> dim, cast, distance );
    }
    if ( dst -> count == 1 )
    {
        td = dst -> td [ 0 ];
        td . dim *= ancestor -> dim;
        return STypesetdecl2Typedecl ( src, self -> dim, schema, & td, cast, distance );
    }

    /* can exit early if only testing castability */
    early = ( cast == NULL && distance == NULL ) ? true : false;

    /* full matrix scan */
    for ( best = ~ 0U, count = src -> count, i = 0; i < count; ++ i )
    {
        uint32_t dist;

        td = src -> td [ i ];
        td . dim *= self -> dim;

        if ( VTypedecl2STypesetdecl ( & td, schema, dst, ancestor -> dim, & td, & dist ) )
        {
            if ( early )
                return true;

            if ( dist < best )
            {
                if ( distance != NULL )
                    * distance = dist;
                if ( dist == 0 )
                {
                    if ( cast != NULL )
                        * cast = td;
                    return true;
                }
                btd = td;
                best = dist;
            }
        }
    }

    if ( ( int32_t ) best > 0 )
    {
        if ( cast != NULL )
            * cast = btd;
        return true;
    }

    return false;
}

LIB_EXPORT bool CC VTypedeclToTypedecl ( const VTypedecl *self, const VSchema *schema,
    const VTypedecl *ancestor, VTypedecl *cast, uint32_t *distance )
{
    if ( self == NULL || schema == NULL || ancestor == NULL )
        return false;

    /* source is typeset */
    if ( self -> type_id >= 0x40000000 )
    {
        /* typeset => typeset */
        if ( ancestor -> type_id >= 0x40000000 )
            return VTypesetdeclToTypesetdecl ( self, schema, ancestor, cast, distance );

        /* typeset => typedecl */
        return VTypesetdeclToTypedecl ( self, schema, ancestor, cast, distance );
    }

    /* ancestor is typeset */
    if ( ancestor -> type_id >= 0x40000000 )
    {
        /* typedecl => typeset */
        return VTypedeclToTypesetdecl ( self, schema, ancestor, cast, distance );
    }

    /* typedecl => typedecl */
    return VTypedecl2Typedecl ( self, schema, ancestor, cast, distance );
}


/* ToType
 *  attempt to cast a typedecl to a size-equivalent ancestor decl
 *
 *  "schema" [ IN ] - schema object that would know about this type
 *
 *  "ancestor" [ IN ] - target type or typeset for cast
 *
 *  "cast" [ OUT ] - return parameter for ancestor decl
 *  valid only if function returns true. if "ancestor" was a typeset,
 *  the value of "cast" will be the closest matching ancestral type.
 *
 *  "distance" [ OUT, NULL OKAY ] - optional linear measure of casting
 *  generations. valid only if function returns true. a value of 0 means
 *  that "self" is a direct match with "ancestor", a value of 1 means that
 *  "self" is an immediate subtype of "ancestor", etc.
 *
 *  returns true if cast succeeded
 */
static
bool VTypedecl2Type ( const VTypedecl *self,
    const VSchema *schema, uint32_t gramps,
    VTypedecl *cast, uint32_t *distance )
{
    const SDatatype *src;
    uint32_t i, dim, type_id = self -> type_id;

    /* cannot produce a valid typedecl if self is "any" or dimension is variable */
    if ( type_id == 0 || self -> dim == 0 )
        return false;

    /* casting to "any" or identity */
    if ( gramps == 0 || type_id == gramps)
    {
        if ( cast != NULL && ( const VTypedecl* ) cast != self )
            * cast = * self;
        if ( distance != NULL )
            * distance = 0;
        return true;
    }

    /* find type */
    src = VSchemaFindTypeid ( schema, type_id );
    if ( src == NULL )
        return false;

    /* cast toward gramps */
    for ( dim = self -> dim, i = 0; src -> id > gramps; ++ i )
    {
        dim *= src -> dim;
        src = src -> super;
        if ( src == NULL || src -> domain == 0 )
            return false;
    }

    /* if not met */
    if ( src -> id != gramps )
        return false;

    /* produce output */
    if ( distance != NULL )
        * distance = i;
                            
    if ( cast != NULL )
    {
        cast -> type_id = gramps;
        cast -> dim = dim;
    }

    return true;
}

static
bool STypesetdecl2Type ( const STypeset *self, uint32_t sdim,
    const VSchema *schema, uint32_t gramps,
    VTypedecl *cast, uint32_t *distance )
{
    bool early;
    VTypedecl btd;
    uint32_t i, count, best;

    /* ambiguous if casting to "any" */
    if ( gramps == 0 )
        return false;

    /* can exit early if only testing castability */
    early = ( cast == NULL && distance == NULL ) ? true : false;

    /* cast all to ancestor */
    for ( best = ~ 0U, count = self -> count, i = 0; i < count; ++ i )
    {
        uint32_t dist;

        VTypedecl td;
        td = self -> td [ i ];
        td . dim *= sdim;

        if ( VTypedecl2Type ( & td, schema, gramps, & td, & dist ) )
        {
            /* if only testing castability */
            if ( early )
                return true;

            /* if match is better */
            if ( dist < best )
            {
                if ( distance != NULL )
                    * distance = dist;
                if ( dist == 0 )
                {
                    if ( cast != NULL )
                        * cast = td;
                    return true;
                }
                btd = td;
                best = dist;
            }
        }
    }

    if ( ( int32_t ) best > 0 )
    {
        if ( cast != NULL )
            * cast = btd;
        return true;
    }

    return false;
}

static
bool VTypesetdeclToType ( const VTypedecl *self, const VSchema *schema,
    uint32_t gramps, VTypedecl *cast, uint32_t *distance )
{
    /* find source typeset */
    const STypeset *src = VSchemaFindTypesetid ( schema, self -> type_id );

    /* not found or empty */
    if ( src == NULL || src -> count == 0 )
        return false;

    /* if has single type */
    if ( src -> count == 1 )
    {
        VTypedecl td = src -> td [ 0 ];
        td . dim *= self -> dim;
        return VTypedecl2Type ( & td, schema, gramps, cast, distance );
    }

    /* perform cast */
    return STypesetdecl2Type ( src, self -> dim, schema, gramps, cast, distance );
}

LIB_EXPORT bool CC VTypedeclToType ( const VTypedecl *self, const VSchema *schema,
    uint32_t ancestor, VTypedecl *cast, uint32_t *distance )
{
    if ( self == NULL || schema == NULL )
        return false;

    /* if casting to typeset */
    if ( ancestor >= 0x40000000 )
    {
        /* convert ancestor to VTypedecl,
           since the operation will be identical
           due to the fact that the typeset contents
           will have dimension, i.e. dimensions must match */
        VTypedecl td;
        td . type_id = ancestor;
        td . dim = 1;

        /* typeset => typeset */
        if ( self -> type_id >= 0x40000000 )
            return VTypesetdeclToTypesetdecl ( self, schema, & td, cast, distance );

        /* typedecl => typeset */
        return VTypedeclToTypesetdecl ( self, schema, & td, cast, distance );
    }

    /* typeset => type */
    if ( self -> type_id >= 0x40000000 )
        return VTypesetdeclToType ( self, schema, ancestor, cast, distance );

    /* typedecl => type */
    return VTypedecl2Type ( self, schema, ancestor, cast, distance );
}

/* CommonAncestor
 *  find a common ancestor between "self" and "peer"
 *  returns distance as sum of distances from each to "ancestor"
 */
static
bool VTypedeclTypedeclCmn ( const VTypedecl *self, const VSchema *schema,
    const VTypedecl *peer, VTypedecl *ancestor, uint32_t *distance )
{
    const SDatatype *at, *bt;
    uint32_t a, b, adim, bdim;

    /* retrieve datatypes */
    at = VSchemaFindTypeid ( schema, self -> type_id );
    if ( at == NULL )
        return false;

    bt = VSchemaFindTypeid ( schema, peer -> type_id );
    if ( bt == NULL )
        return false;

    /* walk each side toward parents */
    for ( a = b = 0, adim = self -> dim, bdim = peer -> dim; at -> id != bt -> id; )
    {
        const SDatatype *dt;
        if ( at -> id > bt -> id )
        {
            ++ a;
            adim *= at -> dim;
            dt = at = at -> super;
        }
        else
        {
            ++ b;
            bdim *= bt -> dim;
            dt = bt = bt -> super;
        }
        if ( dt == NULL || dt -> domain == 0 )
            return false;
    }

    if ( adim == 0 )
    {
        if ( bdim == 0 )
            return false;
        adim = bdim;
    }
    else if ( bdim == 0 )
    {
        bdim = adim;
    }

    /* we have a common parent type, but dimensions must match */
    if ( adim == bdim )
    {
        if ( distance != NULL )
            * distance = a + b;
        if ( ancestor != NULL )
        {
            ancestor -> type_id = at -> id;
            ancestor -> dim = adim;
        }
        return true;
    }

    return false;
}

static
bool VTypedeclSTypesetdeclCmn ( const VTypedecl *self, const VSchema *schema,
    const STypeset *peer, uint32_t bdim, VTypedecl *ancestor, uint32_t *distance )
{
    bool early;
    VTypedecl btd;
    uint32_t i, count, best;

    early = ( ancestor == NULL && distance == NULL ) ? true : false;

    for ( best = ~ 0U, count = peer -> count, i = 0; i < count; ++ i )
    {
        uint32_t dist;

        VTypedecl td = peer -> td [ i ];
        td . dim *= bdim;

        if ( VTypedeclTypedeclCmn ( self, schema, & td, & td, & dist ) )
        {
            if ( early )
                return true;

            if ( dist < best )
            {
                if ( distance != NULL )
                    * distance = dist;
                if ( dist == 0 )
                {
                    if ( ancestor != NULL )
                        * ancestor = td;
                    return true;
                }
                btd = td;
                best = dist;
            }
        }
    }

    if ( ( int32_t ) best > 0 )
    {
        if ( ancestor != NULL )
            * ancestor = btd;
        return true;
    }

    return false;
}

static
bool VTypedeclTypesetCmn ( const VTypedecl *self, const VSchema *schema,
    const VTypedecl *peer, VTypedecl *ancestor, uint32_t *distance )
{
    /* find peer typeset */
    const STypeset *ts = VSchemaFindTypesetid ( schema, peer -> type_id );

    /* if not found or empty */
    if ( ts == NULL || ts -> count == 0 )
        return false;

    /* if has a single type */
    if ( ts -> count == 1 )
    {
        VTypedecl td = ts -> td [ 0 ];
        td . dim *= peer -> dim;
        return VTypedeclTypedeclCmn ( self, schema, & td, ancestor, distance );
    }

    /* find best match */
    return VTypedeclSTypesetdeclCmn ( self, schema, ts, peer -> dim, ancestor, distance );
}

static
bool VTypesetTypesetCmn ( const VTypedecl *self, const VSchema *schema,
    const VTypedecl *peer, VTypedecl *ancestor, uint32_t *distance )
{
    bool early;
    VTypedecl btd, td;
    uint32_t i, count, best;
    const STypeset *ats, *bts;

    /* find self typeset */
    ats = VSchemaFindTypesetid ( schema, self -> type_id );
    if ( ats == NULL || ats -> count == 0 )
        return false;

    /* find peer typeset */
    bts = VSchemaFindTypesetid ( schema, peer -> type_id );
    if ( bts == NULL || bts -> count == 0 )
        return false;

    /* check for simple contents */
    if ( ats -> count == 1 )
    {
        td = ats -> td [ 0 ];
        td . dim *= self -> dim;
        if ( bts -> count == 1 )
        {
            btd = bts -> td [ 0 ];
            btd . dim *= peer -> dim;
            return VTypedeclTypedeclCmn ( & td, schema, & btd, ancestor, distance );
        }
        return VTypedeclSTypesetdeclCmn ( & td, schema, bts, peer -> dim, ancestor, distance );
    }
    if ( bts -> count == 1 )
    {
        td = bts -> td [ 0 ];
        td . dim *= peer -> dim;
        return VTypedeclSTypesetdeclCmn ( & td, schema, ats, self -> dim, ancestor, distance );
    }

    early = ( ancestor == NULL && distance == NULL ) ? true : false;

    for ( best = ~ 0U, count = ats -> count, i = 0; i < count; ++ i )
    {
        uint32_t dist = ~ 0;

        td = ats -> td [ i ];
        td . dim *= self -> dim;

        if ( VTypedeclSTypesetdeclCmn ( & td, schema, bts, peer -> dim, & td, & dist ) )
        {
            if ( early )
                return true;

            if ( dist < best )
            {
                if ( distance != NULL )
                    * distance = dist;
                if ( dist == 0 )
                {
                    if ( ancestor != NULL )
                        * ancestor = td;
                    return true;
                }
                btd = td;
                best = dist;
            }
        }
    }

    if ( ( int32_t ) best > 0 )
    {
        if ( ancestor != NULL )
            * ancestor = btd;
        return true;
    }

    return false;
}

LIB_EXPORT bool CC VTypedeclCommonAncestor ( const VTypedecl *self, const VSchema *schema,
    const VTypedecl *peer, VTypedecl *ancestor, uint32_t *distance )
{
    if ( self == NULL || schema == NULL || peer == NULL )
        return false;

    /* wildcard make it hard to find common ancestor */
    if ( self -> type_id == 0 || peer -> type_id == 0 )
        return VTypedeclToTypedecl ( self, schema, peer, ancestor, distance );

    /* rh type is typeset */
    if ( self -> type_id >= 0x40000000 )
    {
        /* typeset, typeset => parent */
        if ( peer -> type_id >= 0x40000000 )
            return VTypesetTypesetCmn ( self, schema, peer, ancestor, distance );

        /* typeset, typedecl => parent */
        return VTypedeclTypesetCmn ( peer, schema, self, ancestor, distance );
    }

    /* typedecl, typeset => parent */
    if ( peer -> type_id >= 0x40000000 )
        return VTypedeclTypesetCmn ( self, schema, peer, ancestor, distance );

    /* typedecl, typedecl => parent */
    return VTypedeclTypedeclCmn ( self, schema, peer, ancestor, distance );
}

/* Dump
 */
rc_t VTypedeclDump ( const VTypedecl *self, SDumper *b )
{
    rc_t rc;
    uint32_t type_id = self -> type_id;
    if ( type_id < 0x40000000 )
    {
        const SDatatype *dt = VSchemaFindTypeid ( b -> schema, type_id );
        rc = SDatatypeDump ( dt, b );
        if ( rc == 0 && dt != NULL && dt -> id == 0 )
            return 0;
    }
    else if ( type_id < 0x80000000 )
    {
        const STypeset *ts = VSchemaFindTypesetid ( b -> schema, type_id );
        rc = STypesetDump ( ts, b );
    }
    else
    {
#if SLVL >= 3
        const SIndirectType *id = VSchemaFindITypeid ( b -> schema, type_id );
        rc = SIndirectTypeDump ( id, b );
#else
        rc = RC ( rcVDB, rcSchema, rcWriting, rcType, rcUnrecognized );
#endif
    }

    if ( rc == 0 && self -> dim != 1 )
    {
        const char *fmt;
        switch ( SDumperMode ( b ) )
        {
        case sdmCompact:
            fmt = self -> dim ? "[%u]" : "[*]"; break;
        default:
            fmt = self -> dim ? " [ %u ]" : " [ * ]";
        }
        rc = SDumperPrint ( b, fmt, self -> dim );
    }

    return rc;
}
#endif


/*--------------------------------------------------------------------------
 * VTypedesc
 *  describes the fundamental properties of a VTypedecl
 *  replaces the deprecated structure VTypedef
 */

/* Sizeof
 */
LIB_EXPORT uint32_t CC VTypedescSizeof ( const VTypedesc *self )
{
    if ( self == NULL )
        return 0;
    return self -> intrinsic_bits * self -> intrinsic_dim;
}


/*--------------------------------------------------------------------------
 * VFormatdecl
 */
#if SLVL >= 1
LIB_EXPORT rc_t CC VFormatdeclToText ( const VFormatdecl *self,
    const VSchema *schema, char *buffer, size_t bsize )
{
    PLOGMSG ( klogWarn, ( klogWarn, "TDB: $(msg)", "msg=handle VFormatdeclToText " ));    
    return -1;
}

/* ToFormatdecl
 *  attempt to cast a fmtdecl to a size-equivalent ancestor decl
 *
 *  "schema" [ IN ] - schema object that would know about this type
 *
 *  "ancestor" [ IN ] - target fmtdecl or typespec for cast
 *
 *  "cast" [ OUT ] - return parameter for ancestor decl
 *  valid only if function returns true. if "ancestor" contained a typeset,
 *  the value of "cast" will be the closest matching ancestral type.
 *
 *  "distances" [ OUT, NULL OKAY ] - optional linear measure of independent
 *  casting generations for format ( 0 ) and type ( 1 ). valid only if function
 *  returns true. a value of 0 means that "self" is a direct match with "ancestor",
 *  a value of 1 means that "self" is an immediate subtype of "ancestor", etc.
 *
 *  returns true if cast succeeded
 */
static
bool VFormatdeclToFormatdeclInt ( const VFormatdecl *self, const VSchema *schema,
    const VFormatdecl *ancestor, VFormatdecl *cast, uint32_t distances [ 2 ],
    bool ( CC * td2td ) ( const VTypedecl*, const VSchema*, const VTypedecl*, VTypedecl*, uint32_t* ) )
{
    if ( self != NULL && schema != NULL && ancestor != NULL )
    {
        uint32_t i;
        const SFormat *fmt;

        /* just to handle common cases */
        if ( distances != NULL )
            distances [ 0 ] = distances [ 1 ] = 0;

        /* no format on self */
        if ( self -> fmt == 0 )
        {
            /* resulting format is ancestor's */
            if ( cast != NULL )
                cast -> fmt = ancestor -> fmt;

            /* if no ancestor format, or if have type info, perform type cast */
            if ( ancestor -> fmt == 0 || self -> td . type_id != 0 || ancestor -> td . type_id != 0 )
            {
                return ( * td2td ) ( & self -> td, schema,
                    & ancestor -> td, & cast -> td, distances ? & distances [ 1 ] : NULL );
            }

            /* resultant type is undefined */
            if ( cast != NULL )
            {
                cast -> td . type_id = 0;
                cast -> td . dim = 1;
            }

            return true;
        }

        if ( ancestor -> fmt == 0 || ancestor -> fmt == self -> fmt )
        {
            /* resulting format is our own */
            if ( cast != NULL )
                cast -> fmt = self -> fmt;

            /* if have type info, perform type cast */
            if ( self -> td . type_id != 0 || ancestor -> td . type_id != 0 )
            {
                return ( * td2td ) ( & self -> td, schema,
                    & ancestor -> td, & cast -> td, distances ? & distances [ 1 ] : NULL );
            }

            /* resultant type is undefined */
            if ( cast != NULL )
            {
                cast -> td . type_id = 0;
                cast -> td . dim = 1;
            }

            return true;
        }

        /* distinct formats involved */
        fmt = VSchemaFindFmtid ( schema, self -> fmt );
        for ( i = 0; fmt != NULL; ++ i, fmt = fmt -> super )
        {
            if ( fmt -> id == ancestor -> fmt )
            {
                /* successful cast to ancestor */
                if ( cast != NULL )
                    cast -> fmt = ancestor -> fmt;
                if ( distances != NULL )
                    distances [ 0 ] = i;

                /* if have type info, perform type cast */
                if ( self -> td . type_id != 0 || ancestor -> td . type_id != 0 )
                {
                    return ( * td2td ) ( & self -> td, schema,
                        & ancestor -> td, & cast -> td, distances ? & distances [ 1 ] : NULL );
                }

                /* resultant type is undefined */
                if ( cast != NULL )
                {
                    cast -> td . type_id = 0;
                    cast -> td . dim = 1;
                }

                return true;
            }
        }
    }

    return false;
}

LIB_EXPORT bool CC VFormatdeclToFormatdecl ( const VFormatdecl *self,
    const VSchema *schema, const VFormatdecl *ancestor,
    VFormatdecl *cast, uint32_t distances [ 2 ] )
{
    return VFormatdeclToFormatdeclInt ( self, schema,
        ancestor, cast, distances, VTypedeclToTypedecl );
}


/* CommonAncestor
 *  find a common ancestor between "self" and "peer"
 *  returns distance as sum of distances from each to "ancestor"
 */
bool VFormatdeclCommonAncestor ( const VFormatdecl *self, const VSchema *schema,
    const VFormatdecl *peer, VFormatdecl *ancestor, uint32_t distances [ 2 ] )
{
    return VFormatdeclToFormatdeclInt ( self, schema,
        peer, ancestor, distances, VTypedeclCommonAncestor );
}


rc_t VFormatdeclDump ( const VFormatdecl *self, SDumper *b )
{
    rc_t rc;
    uint32_t fmt_id = self -> fmt;

    if ( fmt_id != 0 )
    {
        const SFormat *fmt = VSchemaFindFmtid ( b -> schema, fmt_id );
        rc = SFormatDump ( fmt, b );
        if ( rc != 0 || self -> td . type_id == 0 )
            return rc;

        rc = SDumperWrite ( b, "/", 1 );
        if ( rc != 0 )
            return rc;
    }

    return VTypedeclDump ( & self -> td, b );
}
#endif

/*--------------------------------------------------------------------------
 * STypeset
 */

static
void CC STypesetPopulate ( BSTNode *n, void *data )
{
    const STypesetMbr *mbr = ( const STypesetMbr* ) n;
    STypeset *ts = data;
    ts -> td [ ts -> count ++ ] = mbr -> td;
}

/* Find
 */
STypeset *VSchemaFindTypesetid ( const VSchema *self, uint32_t id )
{
    STypeset *ts = VectorGet ( & self -> ts, id );
    while ( ts == NULL )
    {
        self = self -> dad;
        if ( self == NULL )
            break;
        ts = VectorGet ( & self -> ts, id );
    }
    return ts;
}

/* Mark
 */
void CC STypesetClearMark ( void *item, void *ignore )
{
    STypeset *self = item;
    self -> marked = false;
}

void VSchemaTypeMark ( const VSchema *self, uint32_t type_id )
{
    const SDatatype *dt;
    const STypeset *ts;

    switch ( type_id & 0xC0000000 )
    {
    case 0:
        while ( type_id < VectorStart ( & self -> dt ) )
        {
            self = self -> dad;
            if ( self -> dad == NULL )
                return;
        }

        dt = VectorGet ( & self -> dt, type_id );
        if ( dt != NULL )
            SDatatypeMark ( dt );
        break;

    case 0x40000000:
        while ( type_id < VectorStart ( & self -> ts ) )
        {
            self = self -> dad;
            if ( self -> dad == NULL )
                return;
        }

        ts = VectorGet ( & self -> ts, type_id );
        if ( ts != NULL )
            STypesetMark ( ts, self );
        break;
    }
}

void STypesetMark ( const STypeset *cself, const VSchema *schema )
{
    STypeset *self = ( STypeset* ) cself;
    if ( cself != NULL && ! cself -> marked )
    {
        uint32_t i, count = self -> count;
        self -> marked = true;
        for ( i = 0; i < count; ++ i )
            VSchemaTypeMark ( schema, self -> td [ i ] . type_id );
    }
}

/* Dump
 */
rc_t STypesetDump ( const STypeset *self, struct SDumper *d )
{
    return FQNDump ( self -> name, d );
}

#if SLVL >= 1
bool CC STypesetDefDump ( void *item, void *data )
{
    int i;

    SDumper *b = data;
    const char *tsfmt, *sep, *cls;
    const STypeset *self = ( const void* ) item;

    if ( SDumperMarkedMode ( b ) && ! self -> marked )
        return false;

    switch ( SDumperMode ( b ) )
    {
    case sdmCompact:
        tsfmt = "typeset %N{";
        sep = ",";
        cls = "};";
        break;
    default:
        tsfmt = "typeset %N { ";
        sep = ", ";
        cls = " };\n";
    }


    SDumperSepString ( b, "" );

    b -> rc = SDumperPrint ( b, tsfmt, self -> name );
    for ( i = 0; b -> rc == 0 && i < ( int ) self -> count; ++ i )
    {
        b -> rc = SDumperSep ( b );
        if ( b -> rc == 0 )
            b -> rc = VTypedeclDump ( & self -> td [ i ], b );
        SDumperSepString ( b, sep );
    }
    if ( b -> rc == 0 )
        b -> rc = SDumperPrint ( b, cls );

    if ( b -> rc == 0 )
        b -> rc = AliasDump ( self -> name, b );

    return ( b -> rc != 0 ) ? true : false;
}
#endif

/*--------------------------------------------------------------------------
 * VSchema
 */

/*
 * fqn                = ID [ <nested-name> ]
 * nested-name        = ':' NAME [ <nested-name> ]
 */
static
rc_t nested_name ( const KSymTable *tbl, KTokenSource *src, KToken *t, const SchemaEnv *env )
{
    rc_t rc;
    KSymbol *ns = t -> sym;

    /* look for colon separator */
    if ( next_token ( tbl, src, t ) -> id != eColon )
        return RC ( rcVDB, rcSchema, rcParsing, rcToken, rcIncomplete );

    /* push namespace onto stack */
    rc = KSymTablePushNamespace ( tbl, ns );
    if ( rc == 0 )
    {
        /* look for another namespace */
        if ( next_shallow_token ( tbl, src, t, false ) -> id == eNamespace )
            rc = nested_name ( tbl, src, t, env );

        /* pop namespace */
        KSymTablePopNamespace ( tbl );
    }

    return rc;
}

rc_t next_fqn ( const KSymTable *tbl, KTokenSource *src, KToken *t, const SchemaEnv *env )
{
    rc_t rc;
    KToken t2;
    KTokenSource src2;

    /* looking for a symbolic name */
    if ( t -> sym == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcToken, rcUnexpected );

    /* anything other than a namespace is what we want */
    if ( t -> id != eNamespace )
        return 0;

    /* perform a lookahead */
    t2 = * t;
    src2 = * src;

    /* requires ':' following namespace */
    rc = nested_name ( tbl, & src2, & t2, env );
    if ( rc == 0 )
    {
        KSymbol *ns;
        uint32_t scope;

        /* recognized patern (NS:)+TOKEN
           check that TOKEN is a defined symbol within NS */
        if ( t2 . sym != NULL )
        {
            * t = t2;
            * src = src2;
            return 0;
        }

        /* before returning an error, look harder.
           extended VSchema can create symtab entries
           for namespaces that opaque the parent schema */
        for ( scope = 0, ns = t -> sym; ns != NULL; )
        {
            /* find a deeper symbol */
            ns = KSymTableFindNext ( tbl, ns, & scope );
            if ( ns != NULL && ns -> type == eNamespace )
            {
                t2 = * t;
                src2 = * src;
                t2 . sym = ns;
                rc = nested_name ( tbl, & src2, & t2, env );
                if ( rc != 0 )
                    return rc;
                if ( t2 . sym != NULL )
                {
                    * t = t2;
                    * src = src2;
                    return 0;
                }
            }
        }

        /* could not find anything */
        rc = RC ( rcVDB, rcSchema, rcParsing, rcToken, rcIncomplete );
    }
    return rc;
}

static
rc_t create_fqn_sym ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, uint32_t id, const void *obj )
{
    KToken t2;
    KTokenSource src2 = * src;

    /* need to peek at next token */
    if ( KTokenizerNext ( kDefaultTokenizer, & src2, & t2 ) -> id == eColon )
    {
        /* and again to find ident or name */
        KTokenizerNext ( kDefaultTokenizer, & src2, & t2 );

        /* allow reserved word "view" to be used in schemas <= 1.0 */
        if ( t2 . id == kw_view && ! env -> has_view_keyword )
            t2 . id = eIdent;

        if ( t2 . id == eIdent || t2 . id == eName )
        {
            /* create a namespace and go in */
            KSymbol *ns;
            rc_t rc = KSymTableCreateNamespace ( tbl, & ns, & t -> str );
            if ( rc == 0 )
            {
                rc = KSymTablePushNamespace ( tbl, ns );
                if ( rc == 0 )
                {
                    /* accept ':' NS */
                    * t = t2;
                    * src = src2;

                    /* recurse */
                    rc = create_fqn_sym ( tbl, src, t, env, id, obj );

                    /* exit namespace */
                    KSymTablePopNamespace ( tbl );
                }
            }

            /* always return */
            return rc;
        }
    }

    /* contents of "t" are a proper symbol */
    assert ( t -> id == eIdent || t -> id == eName );
    return KSymTableCreateSymbol ( tbl, & t -> sym, & t -> str, id, obj );
}

static
rc_t enter_namespace ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, uint32_t id, const void *obj )
{
    KSymbol *ns = t -> sym;
    rc_t rc = KSymTablePushNamespace ( tbl, ns );
    if ( rc == 0 )
    {
        if ( next_token ( tbl, src, t ) -> id != eColon )
            rc = KTokenExpected ( t, klogErr, ":" );
        else switch ( next_shallow_token ( tbl, src, t, false ) -> id )
        {
        case eNamespace:
            rc = enter_namespace ( tbl, src, t, env, id, obj );
            break;
        case eIdent:
        case eName:
            rc = create_fqn_sym ( tbl, src, t, env, id, obj );
            break;
        case kw_view:
            if ( ! env -> has_view_keyword )
            {
                rc = create_fqn_sym ( tbl, src, t, env, id, obj );
                break;
            }
            /* no break */
        default:
            if ( t -> id == id )
                rc = SILENT_RC ( rcVDB, rcSchema, rcParsing, rcToken, rcExists );
            else
                rc = RC ( rcVDB, rcSchema, rcParsing, rcToken, rcUnexpected );
        }

        KSymTablePopNamespace ( tbl );
    }
    return rc;
}

rc_t create_fqn ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, uint32_t id, const void *obj )
{
    switch ( t -> id )
    {
    case eNamespace:
        return enter_namespace ( tbl, src, t, env, id, obj );
    case eIdent:
        return create_fqn_sym ( tbl, src, t, env, id, obj );
    default:
        if ( t -> id == id )
            return SILENT_RC ( rcVDB, rcSchema, rcParsing, rcToken, rcExists );
    }

    return RC ( rcVDB, rcSchema, rcParsing, rcToken, rcUnexpected );
}


/*
 * dim                = '[' <uint-expr> ']'
 */
#if SLVL >= 1
rc_t dim ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, uint32_t *dim, bool required )
{
    rc_t rc;

    if ( ! required && t -> id != eLeftSquare )
    {
        * dim = 1;
        return 0;
    }

    /* consume '[' */
    rc = expect ( tbl, src, t, eLeftSquare, "[", true );
    if ( rc == 0 )
    {
        /* assume manifest constant */
        rc = KTokenToU32 ( t, dim );

        /* otherwise, assume some sort of symbolic expression */
        if ( rc != 0 )
        {
            const SExpression *x;
            rc = const_expr ( tbl, src, t, env, self, & x );
            if ( rc == 0 )
            {
                Vector dummy;
                VectorInit ( & dummy, 1, 16 );

                /* should have evaluated to a constant expression */
                rc = eval_uint_expr ( self, x, dim, & dummy );
                SExpressionWhack ( x );

                VectorWhack ( & dummy, NULL, NULL );
            }
            if ( rc != 0 )
                return rc;
        }
    }

    /* close brackets */
    return expect ( tbl, src, next_token ( tbl, src, t ), eRightSquare, "]", true );
}


/*
 * typename           = <fqn>
 */
rc_t type_name ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, uint32_t *id )
{
    const SDatatype *dt;
    const SIndirectType *tp;

    /* consume fully qualified name */
    rc_t rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return rc;

    /* the name must be defined as a data type */
    switch ( t -> id )
    {
    case eDatatype:
        dt = t -> sym -> u. obj;
        * id = dt -> id;
        break;
    case eSchemaType:
        tp = t -> sym -> u. obj;
        * id = tp -> id;
        break;
    default:
        return RC ( rcVDB, rcSchema, rcParsing, rcToken, rcUnexpected );
    }

    return 0;
}


/*
 * typedecl           = <typename> [ <dim> ]
 */
rc_t typedecl ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, VTypedecl *td )
{
    rc_t rc = type_name ( tbl, src, t, env, & td -> type_id );
    if ( rc != 0 )
        return rc;

    next_token ( tbl, src, t );
    return dim ( tbl, src, t, env, self, & td -> dim, false );
}


/*
 * typeset            = <fqn>
 */
rc_t typeset ( const KSymTable *tbl, KTokenSource *src, KToken *t,
   const SchemaEnv *env, uint32_t *id )
{
    const STypeset *ts;

    /* consume fully qualified name */
    rc_t rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return rc;

    /* the name must be defined as a typeset */
    if ( t -> id != eTypeset )
        return RC ( rcVDB, rcSchema, rcParsing, rcToken, rcUnexpected );

    /* retrieve the runtime type id */
    assert ( t -> sym != NULL );
    ts = t -> sym -> u . obj;
    * id = ts -> id;

    return 0;
}

/*
 * typespec           = <typedecl>
 *                    | <typeset> [ <dim> ]
 */
rc_t typespec ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, VTypedecl *td )
{
    const STypeset *ts;
    const SDatatype *dt;
    const SIndirectType *tp;

    /* get a typename or typeset */
    rc_t rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return rc;

    switch ( t -> id )
    {
    case eDatatype:
        dt = t -> sym -> u. obj;
        td -> type_id = dt -> id;
        break;
    case eSchemaType:
        tp = t -> sym -> u. obj;
        td -> type_id = tp -> id;
        break;
    case eTypeset:
        ts = t -> sym -> u. obj;
        td -> type_id = ts -> id;
        break;
    default:
        return RC ( rcVDB, rcSchema, rcParsing, rcToken, rcUnexpected );
    }

    next_token ( tbl, src, t );
    return dim ( tbl, src, t, env, self, & td -> dim, false );
}
#endif


/*
 * fmtname            = <fqn>
 */
rc_t fmtname ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, uint32_t *id )
{
    const SFormat *fmt;

    /* consume fully qualified name */
    rc_t rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return rc;

    /* the name must be defined as a format */
    if ( t -> id != eFormat )
        return RC ( rcVDB, rcSchema, rcParsing, rcToken, rcUnexpected );

    /* retrieve the runtime fmt id */
    assert ( t -> sym != NULL );
    fmt = t -> sym -> u . obj;
    * id = fmt -> id;

    return 0;
}

/*
 * fmtdecl            = <fmtname> [ '/' <typedecl> ]
 *                    | <typedecl>
 */
#if SLVL >= 1
rc_t fmtdecl ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, VFormatdecl *fd )
{
    const SDatatype *dt;
    const SIndirectType *tp;

    /* get a fmtname or typename */
    rc_t rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return rc;

    /* look first for fmtname */
    if ( t -> id == eFormat )
    {
        const SFormat *fmt = t -> sym -> u . obj;
        fd -> fmt = fmt -> id;

        /* unless followed by '/', this is a simple format declaration */
        if ( next_token ( tbl, src, t ) -> id != eFwdSlash )
        {
            fd -> td . type_id = 0;
            fd -> td . dim = 0;
            return 0;
        }

        /* expect a typename */
        rc = next_fqn ( tbl, src, next_token ( tbl, src, t ), env );
        if ( rc != 0 )
            return rc;
    }
    else
    {
        fd -> fmt = 0;
    }

    switch ( t -> id )
    {
    case eDatatype:
        dt = t -> sym -> u. obj;
        fd -> td . type_id = dt -> id;
        break;
    case eSchemaType:
        tp = t -> sym -> u. obj;
        fd -> td . type_id = tp -> id;
        break;
    default:
        return RC ( rcVDB, rcSchema, rcParsing, rcToken, rcUnexpected );
    }

    next_token ( tbl, src, t );
    return dim ( tbl, src, t, env, self, & fd -> td . dim, false );
}

/*
 * fmtspec            = <typespec>
 *                    | <fmtname> [ '/' <typedecl> ]
 */
rc_t fmtspec ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, VFormatdecl *fd )
{
    const SFormat *fmt;
    const STypeset *ts;
    const SDatatype *dt;
    const SIndirectType *tp;

    /* get a fmtname, typename or typeset */
    rc_t rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return rc;

    fd -> fmt = 0;
    switch ( t -> id )
    {
    case eDatatype:
        dt = t -> sym -> u. obj;
        fd -> td . type_id = dt -> id;
        break;
    case eSchemaType:
        tp = t -> sym -> u. obj;
        fd -> td . type_id = tp -> id;
        break;
    case eTypeset:
        ts = t -> sym -> u. obj;
        fd -> td . type_id = ts -> id;
        break;
    case eFormat:
        fmt = t -> sym -> u . obj;
        fd -> fmt = fmt -> id;

        /* unless followed by '/', this is a simple format declaration */
        if ( next_token ( tbl, src, t ) -> id != eFwdSlash )
        {
            fd -> td . type_id = 0;
            fd -> td . dim = 0;
            return 0;
        }
        return typedecl ( tbl, src, t, env, self, & fd -> td );

    default:
        return RC ( rcVDB, rcSchema, rcParsing, rcToken, rcUnexpected );
    }

    next_token ( tbl, src, t );
    return dim ( tbl, src, t, env, self, & fd -> td . dim, false );
}
#endif


/*
 * type-definition    = 'typedef' <typename> <typedef-list>
 * typedef-list       = <typedef-decl> [ ',' <typedef-list> ]
 * typedef-decl       = <fqn> [ <dim> ]
 */
#if SLVL >= 1
rc_t type_definition ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    VTypedecl td;
    uint32_t super_size;
    const SDatatype *super;

    /* expect a typename */
    rc_t rc = type_name ( tbl, src, t, env, & td . type_id );
    if ( rc != 0 )
        return KTokenFailure ( t, klogErr, rc, "type name" );
    if ( td . type_id == 0 )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcType, rcIncorrect );
        return KTokenFailure ( t, klogErr, rc, "illegal base type" );
    }

    /* capture supertype */
    assert ( t -> sym != NULL );
    assert ( t -> sym -> type == eDatatype );
    super = t -> sym -> u . obj;

    /* get supertype size */
    super_size = super -> size;

    do
    {
        KSymbol *sym;
        SDatatype *dt;

        /* expect a fully qualified name */
        rc = create_fqn ( tbl, src, next_token ( tbl, src, t ), env, eDatatype, NULL );
        if ( rc == 0 )
        {
            /* remember symbol */
            sym = t -> sym;

            /* get dimension */
            next_token ( tbl, src, t );
            rc = dim ( tbl, src, t, env, self, & td . dim, false );
            if ( rc != 0 )
                return KTokenFailure ( t, klogErr, rc, "vector dimension" );

            /* allocate a datatype */
            dt = malloc ( sizeof * dt );
            if ( dt == NULL )
            {
                rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
                return KTokenRCExplain ( t, klogInt, rc );
            }

            /* fill it out from super-type */
            dt -> super = super;
            dt -> byte_swap = super -> byte_swap;
            dt -> name = sym;
            dt -> size = super_size * td . dim;
            dt -> dim = td . dim;
            dt -> domain = super -> domain;

            /* insert into type vector */
            rc = VectorAppend ( & self -> dt, & dt -> id, dt );
            if ( rc != 0 )
            {
                free ( dt );
                return KTokenRCExplain ( t, klogInt, rc );
            }

            /* connect symbol */
            sym -> u . obj = dt;
        }

        /* allow a benign redefine */
        else if ( GetRCState ( rc ) != rcExists )
            return KTokenFailure ( t, klogErr, rc, "fully qualified name" );
        else
        {
            /* remember this already defined name */
            sym = t -> sym;
            assert ( sym != NULL && sym -> type == eDatatype );

            /* look for a dimension */
            next_token ( tbl, src, t );
            rc = dim ( tbl, src, t, env, self, & td . dim, false );
            if ( rc != 0 )
                return KTokenFailure ( t, klogErr, rc, "vector dimension" );

            /* peek at existing definition */
            dt = ( SDatatype* ) sym -> u . obj;
            if ( dt -> super != super || dt -> dim != td . dim )
            {
                rc = RC ( rcVDB, rcSchema, rcParsing, rcToken, rcExists );
                return KTokenRCExplain ( t, klogErr, rc );
            }
        }
    }
    while ( t -> id == eComma );

    return expect ( tbl, src, t, eSemiColon, ";", true );
}
#endif


/*
 * format-definition  = 'fmtdef' [ <fmtname> ] <fqn>
 * fmtname            = <fqn>
 */
rc_t format_definition ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    KSymbol *sym;
    SFormat *fmt;
    const SFormat *super;

    /* start with the assumption that the name needs to be created */
    rc_t rc = create_fqn ( tbl, src, t, env, eFormat, NULL );
    if ( rc == 0 )
    {
        super = NULL;
        sym = t -> sym;
        next_token ( tbl, src, t );
    }
    else if ( GetRCState ( rc ) != rcExists )
        return KTokenFailure ( t, klogErr, rc, "fully qualified name" );
    else
    {
        /* could be a superfmt or could be a redefine */
        sym = t -> sym;
        if ( next_token ( tbl, src, t ) -> id != eIdent && t -> sym == NULL )
        {
            /* we will assume it was a redefine of a simple format */
            return expect ( tbl, src, t, eSemiColon, "fully qualified name or ;", true );
        }

        /* has a superfmt */
        super = sym -> u . obj;

        /* do it again */
        rc = create_fqn ( tbl, src, t, env, eFormat, NULL );
        if ( rc != 0 )
        {
            /* may have been a redefine of a path-based format */
            if ( GetRCState ( rc ) != rcExists )
                return KTokenFailure ( t, klogErr, rc, "fully qualified name" );

            return expect ( tbl, src, next_token ( tbl, src, t ),
                eSemiColon, "fully qualified name or ;", true );
        }

        sym = t -> sym;
        next_token ( tbl, src, t );
    }

    /* create a format */
    fmt = malloc ( sizeof * fmt );
    if ( fmt == NULL )
    {
        rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    /* superfmt */
    fmt -> super = super;

    /* link to name */
    fmt -> name = sym;

    /* insert into vector */
    rc = VectorAppend ( & self -> fmt, & fmt -> id, fmt );
    if ( rc != 0 )
    {
        free ( fmt );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    /* link symbol to fmt */
    sym -> u . obj = fmt;

    return expect ( tbl, src, t, eSemiColon, ";", true );
}


/*
 * typeset-definition = 'typeset' <typeset> '{' <typespec-list> '}'
 * typespec-list      = <typespec> [ ',' <typespec-list> ]
 */
#if SLVL >= 1
rc_t typeset_definition ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    BSTree tree;
    KSymbol *sym;
    bool existing;
    uint32_t i, count;

    rc_t rc = create_fqn ( tbl, src, t, env, eTypeset, NULL );
    if ( rc == 0 )
    {
        sym = t -> sym;
        existing = false;
    }
    else if ( GetRCState ( rc ) != rcExists )
        return KTokenFailure ( t, klogErr, rc, "fully qualified name" );
    else
    {
        sym = t -> sym;
        existing = true;
    }

    if ( next_token ( tbl, src, t ) -> id != eLeftCurly )
        return KTokenExpected ( t, klogErr, "{" );

    BSTreeInit ( & tree );
    count = 0;

    do
    {
        STypesetMbr *mbr, *exist;

        /* read typepec */
        VTypedecl td;
        next_token ( tbl, src, t );
        rc = typespec ( tbl, src, t, env, self, & td );
        if ( rc != 0 )
        {
            KTokenFailure ( t, klogErr, rc, "type name, format name or typeset name" );
            break;
        }

        /* if we got a typedecl, add it */
        if ( td . type_id < 0x40000000 )
        {
            mbr = malloc ( sizeof * mbr );
            if ( mbr == NULL )
            {
                rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
                KTokenRCExplain ( t, klogInt, rc );
                break;
            }

            mbr -> td = td;

            /* ignore/allow duplicates */
            if ( BSTreeInsertUnique ( & tree, & mbr -> n,
                 ( BSTNode** ) & exist, STypesetMbrSort ) != 0 )
            {
                free ( mbr );
            }
            else
            {
                ++ count;
            }
        }

        /* got a typeset - add each of its members */
        else
        {
            const STypeset *ts;
            const VSchema *s = self;

            do
            {
                ts = VectorGet ( & s -> ts, td . type_id );
                if ( ts != NULL )
                    break;
                s = s -> dad;
            }
            while ( s != NULL );
            assert ( ts != NULL );

            for ( i = 0; i < ts -> count; ++ i )
            {
                mbr = malloc ( sizeof * mbr );
                if ( mbr == NULL )
                {
                    rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
                    KTokenRCExplain ( t, klogInt, rc );
                    break;
                }

                mbr -> td = ts -> td [ i ];

                /* ignore/allow duplicates */
                if ( BSTreeInsertUnique ( & tree, & mbr -> n,
                     ( BSTNode** ) & exist, STypesetMbrSort ) != 0 )
                {
                    free ( mbr );
                }
                else
                {
                    ++ count;
                }
            }

            if ( rc != 0 )
                break;
        }
    }
    while ( t -> id == eComma );

    if ( rc != 0 )
    {
        BSTreeWhack ( & tree, BSTreeMbrWhack, NULL );
        return rc;
    }

    if ( t -> id != eRightCurly )
        rc = KTokenExpected ( t, klogErr, "}" );
    else
    {
        /* we have a typeset */
        STypeset *ts = malloc ( sizeof * ts - sizeof ts -> td + count * sizeof ts -> td [ 0 ] );
        if ( ts == NULL )
            rc = RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );
        else
        {
            /* initialize */
            ts -> name = sym;
            ts -> count = 0;

            /* copy fmtdecls */
            BSTreeForEach ( & tree, false, STypesetPopulate, ts );
            assert ( ts -> count == count );

            /* have a new one */
            if ( ! existing )
            {
                /* enter it into list */
                sym -> u . obj = ts;
                rc = VectorAppend ( & self -> ts, & ts -> id, ts );
                if ( rc != 0 )
                {
                    free ( ts );
                    KTokenRCExplain ( t, klogInt, rc );
                }
            }

            /* allow benign redefine */
            else
            {
                const STypeset *orig = sym -> u . obj;
                if ( orig -> count != count )
                {
                    rc = RC ( rcVDB, rcSchema, rcParsing, rcToken, rcExists );
                    KTokenRCExplain ( t, klogErr, rc );
                }
                else for ( i = 0; i < count; ++ i )
                {
                    if ( VTypedeclCmp ( & orig -> td [ i ], & ts -> td [ i ] ) != 0 )
                    {
                        rc = RC ( rcVDB, rcSchema, rcParsing, rcToken, rcExists );
                        KTokenRCExplain ( t, klogErr, rc );
                        break;
                    }
                }

                free ( ts );
            }
        }
    }

    BSTreeWhack ( & tree, BSTreeMbrWhack, NULL );

    if ( rc == 0 )
        rc = expect ( tbl, src, next_token ( tbl, src, t ), eSemiColon, ";", false );

    return rc;
}
#endif
