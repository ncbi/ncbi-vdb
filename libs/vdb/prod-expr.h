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

#ifndef _h_prod_expr_
#define _h_prod_expr_

#ifndef _h_vdb_extern_
#include <vdb/extern.h>
#endif

#ifndef _h_cursor_priv_
#include "cursor-priv.h"
#endif

#ifndef _h_prod_priv_
#include "prod-priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * forwards
 */
struct BSTree;
struct KDlset;
struct VSchema;
struct SProduction;
struct SPhysMember;
struct SColumn;
struct SExpression;
struct SFuncExpr;
struct SPhysEncExpr;
struct VLinker;
struct VPhysical;
struct VTable;


/*--------------------------------------------------------------------------
 * VProduction
 */

/* symbol for failed production */
#define FAILED_PRODUCTION ( ( VProduction* ) 1U )


/*--------------------------------------------------------------------------
 * VTypedecl
 */

/* Resolve
 *  ensures typedecl is completely resolved
 */
rc_t VTypedeclResolve ( const VTypedecl *self,
    struct VSchema const *schema, VTypedecl *resolved );


/*--------------------------------------------------------------------------
 * VProdResolve
 */
typedef struct VProdResolve VProdResolve;
struct VProdResolve
{
    /* source of type-related information, functions and structures */
    struct VSchema const *schema;

    /* source of factory functions */
    struct VLinker *ld;
    struct KDlset const *libs;

    /* name of table/view */
    const String * name;

    /* the table whose rowId space we are currently in */
    const struct VTable * primary_table;

    /* the view we are in; NULL if not in a view */
    const struct VView * view;

    /* cursor containing columns, physicals, productions, triggers */
    struct VCursor *curs;

    /* vector of cached named productions */
    VCursorCache *cache;

    /* vector of owned productions */
    Vector *owned;

    /* vector of late-binding expressions */
    Vector *cx_bind;

    /* which chain is being resolved
       chainEncoding    - when going from input to physical
       chainDecoding    - when pulling from physical to output
       chainUncommitted - when resolving trigger and validation expressions
    */
    uint8_t chain;

    /* true if within blob realm */
    bool blobbing;

    /* true if column resolution errors should NOT halt cursor open */
    bool ignore_column_errors;

    /* true if discovering writable columns */
    bool discover_writable_columns;
};


/* ResolveColumn
 *  resolves a column using read/validate expression
 */
rc_t VProdResolveColumn ( const VProdResolve *self,
    VProduction **out, struct SColumn const *scol, bool alt );
rc_t VProdResolveColumnRoot ( const VProdResolve *self,
    VProduction **out, struct SColumn const *scol );
rc_t VProdResolveColumnRead ( const VProdResolve *self,
    VProduction **out, struct SColumn const *scol );


/* ResolvePhysical
 *  resolves a physical column
 */
rc_t VProdResolvePhysical ( const VProdResolve *self,
    struct VPhysical *phys );
rc_t VProdResolvePhysicalRead ( const VProdResolve *self,
    struct VPhysical *phys );


/* ResolveExpr
 *  resolves expression and returns resulting production object
 *
 *  "out" [ IN/OUT, INITIALLY NULL ] - return parameter for production.
 *  assmed to be preset to NULL on input.
 *  invalid on output with non-zero return code
 *  non-NULL on successful resolution with zero return code
 *
 *  "fd" [ IN/OUT ] - resultant type of assignment expression
 *  on input gives acceptance criteria, i.e. a fmtdecl, typeset or wildcard
 *  invalid on output with non-zero return code or NULL "out"
 *  otherwise, gives actual assigned type on output
 *
 *  "desc" [ OUT, NULL OKAY ] - resultant type description of "out"
 *  invalid on non-zero return or NULL "out"
 *  contains description of fd->td on success
 *
 *  "expr" [ IN ] - expression to be evaluated
 *
 *  "casting" [ IN ] - true if performing explicit type casting
 *  see VProdResolveCastExpr for rules
 *
 *  returns non-zero if a non-recoverable error occurs, 0 otherwise.
 *  resolution is only successful on zero return code and non-NULL out.
 */
rc_t VProdResolveExpr ( const VProdResolve *self,
    VProduction **out, VTypedesc *desc, VFormatdecl *fd,
    struct SExpression const *expr, bool casting );

/* resolves any type of SProduction */
rc_t VProdResolveSProduction ( const VProdResolve *self, VProduction **out,
    struct SProduction const *sprod );

/* resolves an SPhysMember symbol */
rc_t VProdResolveSPhysMember ( const VProdResolve *self,
    VProduction **out, struct SPhysMember const *smbr );

/* resolves a script function */
rc_t VProdResolveScriptExpr ( const VProdResolve *self, VProduction **out,
    const VFormatdecl *lval_fd, struct SFuncExpr const *expr );

/* resolves an external function */
rc_t VProdResolveFuncExpr ( const VProdResolve *self, VProduction **out,
    const VFormatdecl *lval_fd, struct SFuncExpr const *expr );

/* resolves physical encoding/decoding expr */
rc_t VProdResolveEncodingExpr ( const VProdResolve *self, VProduction **out,
    VProduction *in, struct SPhysEncExpr const *expr );


#ifdef __cplusplus
}
#endif

#endif /* _h_prod_expr_ */
