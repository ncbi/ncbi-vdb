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

#ifndef _h_schema_expr_
#define _h_schema_expr_

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifndef _h_klib_vector_
#include <klib/vector.h>
#endif

#ifndef _h_vdb_xform_
#include <vdb/xform.h>
#endif

#ifndef _h_atomic32_
#include <atomic32.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * forwards
 */
struct KSymbol;
struct VSchema;
struct SFormat;
struct SDatatype;
struct STypeset;
struct SIndirectType;
struct SPhysical;
struct SDumper;


/*--------------------------------------------------------------------------
 * SExpression
 *  supertype of all expressions
 */
enum
{
    /* a type expression */
    eTypeExpr,

    /* constant or indirect constant expr */
    eConstExpr, eIndirectExpr,

    /* functional param, schema or table production */
    eParamExpr, eProdExpr, eFwdExpr,

    /* a function passed as a parameter */
    eFuncParamExpr,

    /* extern or physical column expression */
    eColExpr, ePhysExpr,

    /* functional expression */
    eFuncExpr, eScriptExpr,

    /* physical encoding expression */
    ePhysEncExpr,

    /* cast operator expression */
    eCastExpr,

    /* unary negation */
    eNegateExpr,

    /* conditional expression */
    eCondExpr,

    /* vector expression */
    eVectorExpr,

    /* table/view member expression */
    eMembExpr
};

typedef struct SExpression SExpression;
struct SExpression
{
    uint32_t var;

    /* this could be KRefcount */
    atomic32_t refcount;
};

/* Whack
 */
void SExpressionWhack ( const SExpression *self );
void CC SExpressionVWhack ( void *item, void *ignore );

/* Mark
 */
void CC SExpressionMark ( void * item, void * data );

/* Dump
 */
rc_t SExpressionDump ( const SExpression *self, struct SDumper *d );
bool SExpressionVDump ( void *item, void *dumper );
bool CC SExpressionListDump ( void *item, void *dumper );
rc_t SExpressionBracketListDump ( const Vector *expr, struct SDumper *d,
    const char *begin, const char *end );


/*--------------------------------------------------------------------------
 * STypeExpr
 *  handles general form of what has been called a typedecl,
 *  with an optional blob format, type, typeset, indirect type,
 *  and a const dimension expression.
 *
 *  when properly resolved, "fd" contains runtime values
 */
typedef struct STypeExpr STypeExpr;
struct STypeExpr
{
    SExpression dad;

    struct SFormat const *fmt;
    struct SDatatype const *dt;
    struct STypeset const *ts;
    struct SIndirectType const *id;
    const SExpression *dim;

    VFormatdecl fd;
    bool resolved;
};


/* Resolve
 *  resolve type expression to either a VTypedecl or VFormatdecl
 */
rc_t STypeExprResolveAsTypedecl ( const STypeExpr *self,
    struct VSchema const *schema, VTypedecl *td, Vector *cx_bind );
rc_t STypeExprResolveAsFormatdecl ( const STypeExpr *self,
    struct VSchema const *schema, VFormatdecl *fd, Vector *cx_bind );

/* Mark
 */
void STypeExprMark ( const STypeExpr *self, struct VSchema const *schema );


/*--------------------------------------------------------------------------
 * SConstExpr
 *  handles a vector of constant values
 *  evaluated from manifest or direct symbolic constants
 *
 *  "td" holds both an appropriate constant type
 *  as well as vector dimension
 *
 *  when td.type_id is utf8, dim gives
 *  the number of bytes rather than characters.
 */
typedef struct SConstExpr SConstExpr;
struct SConstExpr
{
    SExpression dad;

    VTypedecl td;
    union
    {
        /* bool */
        bool b [ 8 ];

        /* integer */
        int8_t i8 [ 8 ];
        int16_t i16 [ 4 ];
        int32_t i32 [ 2 ];
        int64_t i64 [ 1 ];

        /* unsigned integer */
        uint8_t u8 [ 8 ];
        uint16_t u16 [ 4 ];
        uint32_t u32 [ 2 ];
        uint64_t u64 [ 1 ];

        /* float */
        float f32 [ 2 ];
        double f64 [ 1 ];

        /* ascii */
        char ascii [ 8 ];

        /* unicode */
        char utf8 [ 8 ];
        uint16_t utf16 [ 4 ];
        uint32_t utf32 [ 2 ];

    } u;
};


/* Resolve
 *  return constant expression as a vector of indicated type
 */
rc_t SConstExprResolveAsBool ( const SConstExpr *self,
    struct VSchema const *schema, bool *b, uint32_t capacity );
rc_t SConstExprResolveAsI8 ( const SConstExpr *self,
    struct VSchema const *schema, int8_t *i8, uint32_t capacity );
rc_t SConstExprResolveAsI16 ( const SConstExpr *self,
    struct VSchema const *schema, int16_t *i16, uint32_t capacity );
rc_t SConstExprResolveAsI32 ( const SConstExpr *self,
    struct VSchema const *schema, int32_t *i32, uint32_t capacity );
rc_t SConstExprResolveAsI64 ( const SConstExpr *self,
    struct VSchema const *schema, int64_t *i64, uint32_t capacity );
rc_t SConstExprResolveAsU8 ( const SConstExpr *self,
    struct VSchema const *schema, uint8_t *u8, uint32_t capacity );
rc_t SConstExprResolveAsU16 ( const SConstExpr *self,
    struct VSchema const *schema, uint16_t *u16, uint32_t capacity );
rc_t SConstExprResolveAsU32 ( const SConstExpr *self,
    struct VSchema const *schema, uint32_t *u32, uint32_t capacity, Vector *cx_bind );
rc_t SConstExprResolveAsU64 ( const SConstExpr *self,
    struct VSchema const *schema, uint32_t *u64, uint32_t capacity );
rc_t SConstExprResolveAsF32 ( const SConstExpr *self,
    struct VSchema const *schema, float *f32, uint32_t capacity );
rc_t SConstExprResolveAsF64 ( const SConstExpr *self,
    struct VSchema const *schema, double *f64, uint32_t capacity );
rc_t SConstExprResolveAsAscii ( const SConstExpr *self,
    struct VSchema const *schema, char *ascii, size_t capacity );
rc_t SConstExprResolveAsUTF8 ( const SConstExpr *self,
    struct VSchema const *schema, char *utf8, size_t bytes );
rc_t SConstExprResolveAsUTF16 ( const SConstExpr *self,
    struct VSchema const *schema, uint16_t *utf16, uint32_t capacity );
rc_t SConstExprResolveAsUTF32 ( const SConstExpr *self,
    struct VSchema const *schema, uint32_t *utf32, uint32_t capacity );


/*--------------------------------------------------------------------------
 * SSymExpr
 *  holds a symbolic object reference
 *  used for productions and columns and indirect constants
 *  the productions and columns may be forward declarations,
 *  in which case the symbol will point to NULL when dereferenced.
 */
typedef struct SSymExpr SSymExpr;
struct SSymExpr
{
    SExpression dad;
    struct KSymbol const *_sym;
    bool alt;
};

/* Make
 *  used when creating implicit expressions
 */
rc_t SSymExprMake ( const SExpression **xp,
    struct KSymbol const *sym, uint32_t var );

/* Mark
 */
void SSymExprMark ( const SSymExpr *self, struct VSchema const *schema );


/*--------------------------------------------------------------------------
 * SFuncExpr
 *  gathers schema, factory and functional parameter expressions
 *  locates and dereferences function or script by name and version
 */
typedef struct SFuncExpr SFuncExpr;
struct SFuncExpr
{
    SExpression dad;

    /* some sort of expression */
    struct SFunction const *func;

    /* schema type and constant params */
    Vector schem;

    /* factory params */
    Vector pfact;

    /* function params */
    Vector pfunc;

    /* requested version */
    uint32_t version;
    bool version_requested;

    /* for purposes of obtaining column info
       in the absence of metadata */
    bool untyped;
};

/* Mark
 */
void SFuncExprMark ( const SFuncExpr *self, struct VSchema const *schema );


/*--------------------------------------------------------------------------
 * SPhysEncExpr
 *
 */
typedef struct SPhysEncExpr SPhysEncExpr;
struct SPhysEncExpr
{
    SExpression dad;

    /* physical encoding */
    struct SPhysical const *phys;

    /* schema type and constant params */
    Vector schem;

    /* factory params */
    Vector pfact;

    /* requested version */
    uint32_t version;
    bool version_requested;
};

/* ImplicitPhysEncExpr
 *  create expression object
 */
rc_t VSchemaImplicitPhysEncExpr ( VSchema *self, VTypedecl *td,
    const SExpression **expr, const char *text, const char *ctx );

/* Mark
 */
void SPhysEncExprMark ( const SPhysEncExpr *self,
    struct VSchema const *schema );


/*--------------------------------------------------------------------------
 * SUnaryExpr
 *  applies a unary expression to further
 */
typedef struct SUnaryExpr SUnaryExpr;
struct SUnaryExpr
{
    SExpression dad;
    const SExpression *expr;
};


/*--------------------------------------------------------------------------
 * SBinExpr
 *  connects two expression by means of an operator
 *  the operator is implicit in expression variant
 *
 *  e.g. eCastExpr where left = STypeExpr, right = SExpression
 */
typedef struct SBinExpr SBinExpr;
struct SBinExpr
{
    SExpression dad;
    const SExpression *left;
    const SExpression *right;
};


/*--------------------------------------------------------------------------
 * SVectExpr
 *  gathers multiple expressions connect by means of an operator
 *  the operator is implicit in expression variant
 *
 *  e.g. eCondExpr where operator is '|', or
 *  a constant vector where the operator is ','
 */
typedef struct SVectExpr SVectExpr;
struct SVectExpr
{
    SExpression dad;
    Vector expr;
};

/*--------------------------------------------------------------------------
 * SMembExpr
 *  a member (column or production) a view parameter (a table or a view)
 *  represents an "object . member" expression, where "object" is a parameter
 *  of a directly enclosing view, wich an optional pivot into the object's row-id space.
 */
typedef struct SMembExpr SMembExpr;
struct SMembExpr
{
    SExpression dad;
    const struct SView * view;
    uint32_t paramId;
    const struct KSymbol * member;

    const SExpression * rowId; /* optional row-id in param's row-id space */
};

#ifdef __cplusplus
}
#endif

#endif /* _h_schema_expr_ */
