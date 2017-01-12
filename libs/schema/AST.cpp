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

#include "AST.hpp"

#include <klib/symbol.h>
#include <klib/printf.h>
#include <klib/rc.h>

// hide an unfortunately named C function typename()
#define typename __typename
#include "../vdb/schema-parse.h"
#undef typename
#include "../vdb/dbmgr-priv.h"
#include "../vdb/schema-priv.h"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-ast-tokens.h"

using namespace std;

// AST

SchemaToken st = { PT_EMPTY, NULL, 0, 0, 0 };

AST :: AST ()
: ParseTree ( st )
{
}

AST :: AST ( const Token * p_token )
: ParseTree ( * p_token )
{
}

void
AST :: AddNode ( AST * p_child )
{
    AddChild ( p_child );
}

void
AST :: AddNode ( const Token * p_child )
{
    AddChild ( new AST ( p_child ) );
}

// ASTBuilder

ASTBuilder :: ASTBuilder ()
:   m_debug ( false ),
    m_intrinsic ( 0 ),
    m_schema ( 0 )
{
    VectorInit ( & m_errors, 0, 1024 );

    rc_t rc = VSchemaMakeIntrinsic ( & m_intrinsic );
    if ( rc != 0 )
    {
        ReportError ( "VSchemaMakeIntrinsic failed: %R", rc  );
        m_intrinsic = 0;
    }
    else
    {
        rc = KSymTableInit ( & m_symtab, 0 );
        if ( rc != 0 )
        {
            ReportError ( "KSymTableInit failed: %R", rc );
            VSchemaRelease ( m_intrinsic );
            m_intrinsic = 0;
        }
    }
}


void
ASTBuilder :: DebugOn ()
{
    m_debug = true;
}

static
void CC
WhackMessage ( void *item, void *data )
{
    free ( item );
}

ASTBuilder :: ~ASTBuilder ()
{
    KSymTableWhack ( & m_symtab);
    VSchemaRelease ( m_schema );
    VSchemaRelease ( m_intrinsic );
    VectorWhack ( & m_errors, WhackMessage, 0 );
}

void
ASTBuilder :: ReportError ( const char* p_fmt, ... )
{
    const unsigned int BufSize = 1024;
    char buf [ BufSize ];

    va_list args;
    va_start ( args, p_fmt );
    string_vprintf ( buf, BufSize, 0, p_fmt, args );
    va_end ( args );

    VectorAppend ( & m_errors, 0, string_dup_measure ( buf, 0 ) );
}

void
ASTBuilder :: ReportError ( const char* p_msg, const AST_FQN& p_fqn )
{
    char buf [ 1024 ];
    p_fqn . GetFullName ( buf, sizeof buf );
    ReportError ( "%s: '%s'", p_msg, buf ); //TODO: add location of the original declaration
}

bool
ASTBuilder :: Init()
{
    if ( m_schema != 0 )    // in case we are called more than once
    {
        KSymTableWhack ( & m_symtab);
        VSchemaRelease ( m_schema );
    }
    rc_t rc = VSchemaMake ( & m_schema, m_intrinsic );
    if ( rc != 0 )
    {
        ReportError ( "VSchemaMake failed: %R", rc );
        m_schema = 0;
        return false;
    }

    rc = init_symtab ( & m_symtab, m_schema ); // this pushes the global scope
    if ( rc != 0 )
    {
        ReportError ( "VSchemaMake failed: %R", rc );
        return false;
    }

    return true;
}

AST *
ASTBuilder :: Build ( const ParseTree& p_root )
{
    if ( m_intrinsic == 0 || ! Init () )
    {
        return 0;
    }

    AST* ret = 0;
    AST_debug = m_debug;
    ParseTreeScanner scanner ( p_root );
    if ( AST_parse ( ret, * this, scanner ) == 0 )
    {
        return ret;
    }

    //TODO: report error(s)
    return 0;
}

const KSymbol*
ASTBuilder :: CreateFqnSymbol ( const AST_FQN& p_fqn, uint32_t p_type, const void * p_obj )
{
    rc_t rc = 0;
    uint32_t count = p_fqn . NamespaceCount ();
    for ( uint32_t i = 0 ; i < count; ++ i )
    {
        String name;
        StringInitCString ( & name, p_fqn . GetChild ( i ) -> GetTokenValue () );
        KSymbol *ns;
        rc = KSymTableCreateNamespace ( & m_symtab, & ns, & name );
        if ( rc == 0 )
        {
            rc = KSymTablePushNamespace ( & m_symtab, ns );
            if ( rc != 0 )
            {
                ReportError ( "KSymTablePushNamespace failed: %R", rc );
            }
        }
        else
        {
            ReportError ( "KSymTableCreateNamespace failed: %R", rc );
        }
    }

    KSymbol* ret = 0;
    if ( rc == 0 )
    {
        String name;
        p_fqn . GetIdentifier ( name );
        rc = KSymTableCreateSymbol ( & m_symtab, & ret, & name, p_type, p_obj );
        if ( GetRCState ( rc ) == rcExists )
        {
            ReportError ( "Object already declared", p_fqn );
        }
        else if ( rc != 0 )
        {
            ReportError ( "KSymTableCreateSymbol failed: %R", rc );
        }
    }

    for ( uint32_t i = 0 ; i < count; ++ i )
    {
        KSymTablePopNamespace ( & m_symtab );
    }

    return ret;
}

const KSymbol*
ASTBuilder :: Resolve ( const AST_FQN& p_fqn, bool p_reportUnknown )
{
    uint32_t count = p_fqn . ChildrenCount ();
    assert ( count > 0 );
    bool ns_resolved = true;
    for ( uint32_t i = 0 ; i < count - 1; ++ i )
    {
        String name;
        StringInitCString ( & name, p_fqn . GetChild ( i ) -> GetTokenValue () );
        KSymbol *ns;
        if ( i == 0 )
        {
            ns = KSymTableFindGlobal ( & m_symtab, & name );
        }
        else
        {
            ns = KSymTableFind ( & m_symtab, & name );
        }
        if ( ns == 0 )
        {
            if ( p_reportUnknown )
            {
                ReportError ( "Namespace not found: %S", & name );
            }
            count = i;
            ns_resolved = false;
            break;
        }

        rc_t rc = 0;
        if ( rc == 0 )
        {
            rc = KSymTablePushNamespace ( & m_symtab, ns );
            if ( rc != 0 )
            {
                ReportError ( "KSymTablePushNamespace failed: %R", rc );
                count = i;
                ns_resolved = false;
                break;
            }
        }
    }

    KSymbol * ret = 0;
    if ( ns_resolved )
    {
        const char* ident = p_fqn . GetChild ( count - 1 ) -> GetTokenValue ();
        String name;
        StringInitCString ( & name, ident );
        ret = KSymTableFind ( & m_symtab, & name );
        if ( ret == 0 && p_reportUnknown )
        {
            ReportError ( "Undeclared identifier", p_fqn ); //TODO: add location
        }
    }

    if ( count > 0 )
    {
        for ( uint32_t i = 0 ; i < count - 1; ++ i )
        {
            KSymTablePopNamespace ( & m_symtab );
        }
    }

    return ret;
}

void
ASTBuilder :: DeclareType ( const AST_FQN& p_fqn, const KSymbol& p_super, const AST* p_dimension )
{
    assert ( p_dimension == 0 ); //TODO
    const uint32_t dimension = 1;

    /* allocate a datatype */
    SDatatype * dt = static_cast < SDatatype * > ( malloc ( sizeof * dt ) ); // VSchema's tables dispose of objects with free()
    if ( dt == 0 )
    {
        ReportError ( "malloc failed: %R", RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted ) );
    }
    else
    {
        /* insert into type vector */
        rc_t rc = VectorAppend ( & m_schema -> dt, & dt -> id, dt );
        if ( rc != 0 )
        {
            ReportError ( "VectorAppend failed: %R", rc );
            free ( dt );
        }
        else
        {
            /* capture supertype */
            assert ( p_super . type == eDatatype );
            const SDatatype * super = static_cast < const SDatatype * > ( p_super . u . obj );

            // create a symtab entry
            const KSymbol* symbol = CreateFqnSymbol ( p_fqn, eDatatype, dt ); // will add missing namespaces to symtab
            if ( symbol != 0 )
            {
                /* fill it out from super-type */
                dt -> super     = super;
                dt -> byte_swap = super -> byte_swap;
                dt -> name      = symbol;
                dt -> size      = super -> size * dimension;
                dt -> dim       = dimension;
                dt -> domain    = super -> domain;
            }
        }
    }
}

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

static
void CC STypesetPopulate ( BSTNode *n, void *data )
{
    const STypesetMbr *mbr = ( const STypesetMbr* ) n;
    STypeset *ts = static_cast < STypeset * > ( data );
    ts -> td [ ts -> count ++ ] = mbr -> td;
}

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

void
ASTBuilder :: DeclareTypeSet ( const AST_FQN & p_fqn, const BSTree & p_types, uint32_t p_typeCount )
{
    STypeset *ts = static_cast < STypeset * > ( malloc ( sizeof * ts - sizeof ts -> td + p_typeCount * sizeof ts -> td [ 0 ] ) );
    if ( ts == 0 )
    {
        ReportError ( "malloc failed: %R", RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted ) );
    }
    else
    {
        ts -> count = 0;
        BSTreeForEach ( & p_types, false, STypesetPopulate, ts );

        rc_t rc = VectorAppend ( & m_schema -> ts, & ts -> id, ts );
        if ( rc != 0 )
        {
            ReportError ( "VectorAppend failed: %R", rc );
            free ( ts );
        }
        else
        {
            const KSymbol* symbol = CreateFqnSymbol ( p_fqn, eTypeset, ts ); // will add missing namespaces to symtab
            ts -> name = symbol;
            ts -> count = p_typeCount;
        }
    }
}

// AST nodes

AST_Schema :: AST_Schema ()
:   m_versMajor ( 0 ),
    m_versMinor ( 0 )
{
}


AST_Schema :: AST_Schema ( const Token * p_token, AST* p_decls /*NULL OK*/, unsigned int p_version )
:   AST ( p_token ),
    m_versMajor ( p_version ),
    m_versMinor ( 0 )
{
    if ( p_decls != 0 )
    {
        MoveChildren ( * p_decls );
        delete p_decls;
    }
}

void
AST_Schema :: SetVersion ( const char* ) // version specified as n.m; checked for valid version number here
{
    //TODO
}

AST *
ASTBuilder :: TypeDef ( const Token * p_token, AST_FQN* p_baseType, AST* p_newTypes )
{   //TODO: do we need to keep all these subtrees beyond the population of symtab?
    AST * ret = new AST ( p_token );
    assert ( p_baseType != 0 );
    ret -> AddNode ( p_baseType );
    assert ( p_newTypes != 0 );
    ret -> AddNode ( p_newTypes );

    const KSymbol * baseType = Resolve ( * p_baseType ); // will report unknown name
    if ( baseType != 0 )
    {
        if ( baseType -> type != eDatatype )
        {
            ReportError ( "Not a datatype: '%s'", p_baseType -> GetChild ( 0 ) -> GetTokenValue () ); //TODO: add location
            //TODO: recover? pretend it is "any"?
        }
        else
        {
            uint32_t count = p_newTypes -> ChildrenCount ();
            for ( uint32_t i = 0; i < count; ++i )
            {
                const AST * newType = p_newTypes -> GetChild ( i );
                if ( newType -> GetToken () . GetType () == PT_IDENT )
                {
                    const AST_FQN & fqn = dynamic_cast < const AST_FQN & > ( * newType );
                    DeclareType ( fqn, * baseType, 0 ); // will report duplicate definition
                }
                else // fqn dim
                {   //TODO
                }
            }
        }
    }
    return ret;
}

AST *
ASTBuilder :: ArrayDef ( const Token* p_token, AST_FQN* p_baseType, AST* p_dim )
{
    AST * ret = new AST ( p_token );
    assert ( p_baseType != 0 );
    ret -> AddNode ( p_baseType );
    assert ( p_dim != 0 );
    ret -> AddNode ( p_dim );
    //TBD
    return ret;
}

static
bool
TypeSetAddType ( ASTBuilder & p_builder, BSTree & p_tree, const VTypedecl & p_type, uint32_t & p_typeCount )
{
    STypesetMbr * mbr = static_cast < STypesetMbr * > ( malloc ( sizeof * mbr ) );
    if ( mbr == 0 )
    {
        p_builder . ReportError ( "malloc failed: %R", RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted ) );
        return false;
    }

    mbr -> td = p_type;

    /* ignore/allow duplicates */
    BSTNode * exist;
    if ( BSTreeInsertUnique ( & p_tree, & mbr -> n, & exist, STypesetMbrSort ) != 0 )
    {
        free ( mbr );
    }
    else
    {
        ++ p_typeCount;
    }
    return true;
}

AST *
ASTBuilder :: TypeSet ( const Token* p_token, AST_FQN * p_name, AST * p_typeSpecs )
{
    AST * ret = new AST ( p_token );
    assert ( p_name != 0 );
    ret -> AddNode ( p_name );
    assert ( p_typeSpecs != 0 );
    ret -> AddNode ( p_typeSpecs );

    const KSymbol * existing = Resolve ( * p_name, false );

    // traverse p_typeSpecs, add to tree
    BSTree tree;
    BSTreeInit ( & tree );

    uint32_t typeCount = 0;
    uint32_t count = p_typeSpecs -> ChildrenCount ();
    for ( uint32_t i = 0; i < count; ++i )
    {
        const AST * spec = p_typeSpecs -> GetChild ( i );
        if ( spec -> GetToken () . GetType () == PT_IDENT )
        {   // scalar
            const AST_FQN & fqn = dynamic_cast < const AST_FQN & > ( * spec );
            const KSymbol * type = Resolve ( fqn ); // will report unknown name
            if ( type != 0 )
            {
                switch ( type -> type )
                {
                case eDatatype:
                    {
                        VTypedecl td;
                        const SDatatype * typeDef = static_cast < const SDatatype * > ( type -> u . obj );

                        td . type_id = typeDef -> id;
                        td . dim = 1;
                        if ( ! TypeSetAddType ( * this, tree, td, typeCount ) )
                        {
                            goto EXIT;
                        }
                    }
                    break;
                case eTypeset:
                    {
                        const STypeset * typeset = static_cast < const STypeset * > ( type -> u . obj );
                        for ( uint16_t j = 0; j < typeset -> count; ++j )
                        {
                            if ( ! TypeSetAddType ( *this, tree, typeset -> td [ j ], typeCount ) )
                            {
                                goto EXIT;
                            }
                        }
                    }
                    break;
                default:
                    {
                        ReportError ( "Not a datatype", fqn );
                    }
                    continue;
                }
            }
        }
        else // fqn dim
        {   //TODO
        }
    }

    if ( existing != 0 )
    {
        if ( existing -> type != eTypeset )
        {
            ReportError ( "Already declared and is not a typeset", * p_name );
        }
        else
        {   // allow benign redefine
            const STypeset * orig = static_cast < const STypeset * > ( existing -> u . obj );
            if ( orig -> count != typeCount )
            {
                ReportError ( "Typeset already declared differently", * p_name );
            }
            else
            {
                BSTNode* node = BSTreeFirst ( &tree );
                for ( uint32_t i = 0; i < typeCount; ++ i )
                {
                    assert ( node != 0 );
                    STypesetMbr * mbr = reinterpret_cast < STypesetMbr * > ( node );
                    if ( VTypedeclCmp ( & orig -> td [ i ], & mbr -> td ) != 0 )
                    {
                        ReportError ( "Typeset already declared differently", * p_name );
                        break;
                    }
                    node = BSTNodeNext ( node );
                }
            }
        }
    }
    else
    {
        DeclareTypeSet ( * p_name, tree, typeCount );
    }

EXIT:
    BSTreeWhack ( & tree, BSTreeMbrWhack, NULL );

    return ret;
}

AST *
ASTBuilder :: FmtDef ( const Token* p_token, AST_FQN* p_fqn, AST_FQN* p_super_opt )
{
    AST * ret = new AST ( p_token );
    assert ( p_fqn != 0 );
    ret -> AddNode ( p_fqn );
    if ( p_super_opt != 0 )
    {
        ret -> AddNode ( p_super_opt );
    }

    SFormat * fmt = static_cast < SFormat * > ( malloc ( sizeof * fmt ) );
    if ( fmt == NULL )
    {
        ReportError ( "malloc failed: %R", RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted ) );
    }
    else
    {
        // superfmt
        fmt -> super = 0;
        if ( p_super_opt != 0 )
        {
            const KSymbol* super = Resolve ( * p_super_opt ); // will report undefined
            if ( super != 0 )
            {
                if ( super -> type != eFormat )
                {
                    ReportError ( "Not a format", * p_super_opt );
                    free ( fmt );
                    return ret;
                }
                fmt -> super = static_cast < const SFormat * > ( super -> u . obj );
            }
        }

        /* insert into vector */
        rc_t rc = VectorAppend ( & m_schema -> fmt, & fmt -> id, fmt );
        if ( rc != 0 )
        {
            ReportError ( "VectorAppend failed: %R", rc );
            free ( fmt );
        }
        else
        {   // create a symtab entry, link fmt to it
            fmt -> name = CreateFqnSymbol ( * p_fqn, eFormat, fmt ); // will add missing namespaces to symtab
        }
    }

    return ret;
}



// AST_FQN

AST_FQN :: AST_FQN ( const Token* p_token )
: AST ( p_token )
{
}

uint32_t
AST_FQN :: NamespaceCount() const
{
    uint32_t count = ChildrenCount ();
    return count > 0 ? ChildrenCount () - 1 : 0;
}

void
AST_FQN :: GetIdentifier ( String & p_str ) const
{
    uint32_t count = ChildrenCount ();
    if ( count > 0 )
    {
        StringInitCString ( & p_str, GetChild ( count - 1 ) -> GetTokenValue () );
    }
    else
    {
        CONST_STRING ( & p_str, "" );
    }
}

void
AST_FQN :: GetFullName ( char* p_buf, size_t p_bufSize ) const
{
    GetPartialName ( p_buf, p_bufSize, ChildrenCount () );
}

void
AST_FQN :: GetPartialName ( char* p_buf, size_t p_bufSize, uint32_t p_lastMember ) const
{
    uint32_t count = ChildrenCount ();
    if ( p_lastMember < count )
    {
        count = p_lastMember + 1;
    }
    size_t offset = 0;
    for ( uint32_t i = 0 ; i < count; ++ i )
    {
        size_t num_writ;
        rc_t rc = string_printf ( p_buf + offset, p_bufSize - offset - 1, & num_writ, "%s%s",
                                  GetChild ( i ) -> GetTokenValue (),
                                  i == count - 1 ? "" : ":" );
        offset += num_writ;
        if ( rc != 0 )
        {
            break;
        }
    }

    p_buf [ p_bufSize ] = 0;
}

