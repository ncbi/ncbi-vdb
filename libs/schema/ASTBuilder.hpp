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

#ifndef _hpp_ASTBuilder_
#define _hpp_ASTBuilder_

#include <cstring>

#include <kfc/callconv.h>
#include <klib/rc.h>

#include "AST.hpp"
#include "ErrorReport.hpp"

struct KSymbol;
struct BSTree;
struct STypeExpr;
struct SFunction;
struct SFormParmlist;
struct SIndirectType;
struct SIndirectConst;
struct STable;
struct SColumn;
struct SExpression;
struct SDatabase;
struct VTypedecl;
struct SNameOverload;
struct KFile;
struct SPhysMember;
struct SView;

namespace ncbi
{
    namespace SchemaParser
    {
        class ASTBuilder
        {
        public:
            ASTBuilder ( VSchema * schema );
            ~ASTBuilder ();

            void AddIncludePath ( const char * path );

            AST* Build ( const ParseTree& p_root, const char * p_sourceFileName = "", bool p_debugParse = false );

            const KSymbol* Resolve ( const AST_FQN& p_fqn, bool p_reportUnknown = true );
            KSymbol* Resolve ( const Token :: Location & p_loc, const char* p_ident, bool p_reportUnknown = true );

            uint32_t IntrinsicTypeId ( const char * p_type ) const;

            void ReportError ( const Token :: Location & p_loc, const char * p_msg );
            void ReportError ( const Token :: Location & p_loc, const char * p_msg, const String & p_str );
            void ReportError ( const Token :: Location & p_loc, const char * p_msg, const char * p_str );
            void ReportError ( const Token :: Location & p_loc, const char * p_msg, int64_t p_val );
            void ReportError ( const char * p_msg, const AST_FQN & p_fqn ); // use location of p_fqn
            void ReportInternalError ( const char * p_msg, const char * p_source = "" ); // no line/col
            void ReportRc ( const char * p_msg, rc_t );

            // error list is cleared by a call to Build
            uint32_t GetErrorCount() const { return m_errors . GetCount (); }
            const char* GetErrorMessage ( uint32_t p_idx ) const { return m_errors . GetMessageText ( p_idx ); }
            const ErrorReport & GetErrors () const { return m_errors; }

            // uses malloc(); reports allocation failure
            template < typename T > T* Alloc ( size_t p_size = sizeof ( T ) );

            const VSchema * GetSchema () const { return m_schema; }
            VSchema * GetSchema () { return m_schema; }

            const KSymTable & GetSymTab () const { return m_symtab; }
            KSymTable & GetSymTab () { return m_symtab; }

            // used to make sure view and table elements do not clash on their context Ids
            uint32_t NextContextId() { return m_nextContextId++; }

        public:
            // AST node creation methods for use from bison
            AST * TypeDef ( const Token *, AST_FQN * baseType, AST * newTypes );
            AST * TypeSet ( const Token *, AST_FQN * name, AST * typeSpecs );
            AST * FmtDef  ( const Token *, AST_FQN * name, AST_FQN * super_opt );
            AST * ConstDef  ( const Token *, AST * type, AST_FQN * name, AST_Expr * expr );
            AST * AliasDef  ( const Token *, AST_FQN * name, AST_FQN * newName );
            AST * UntypedFunctionDecl ( const Token *, AST_FQN * name );
            AST * RowlenFunctionDecl ( const Token *, AST_FQN * name );
            AST * FunctionDecl ( const Token *,
                                 bool           script,
                                 AST *          schema,
                                 AST *          returnType,
                                 AST_FQN *      name,
                                 AST  *         fact,
                                 AST *          params,
                                 AST *          prologue );
            AST * PhysicalDecl ( const Token *, AST * schema, AST * returnType, AST_FQN * name, AST * fact, AST * body );
            AST * TableDef ( const Token *, AST_FQN * name, AST * parents, AST * body );
            AST * DatabaseDef ( const Token *, AST_FQN * name, AST * parent, AST * body );
            AST * Include ( const Token *, const Token * filename );
            AST * ViewDef ( const Token *, AST_FQN * name, AST * params, AST * parents, AST * body );

        public: // schema object construction helpers
            const KSymbol* CreateFqnSymbol ( const AST_FQN& fqn, uint32_t type, const void * obj );

            KSymbol * CreateLocalSymbol ( const AST & p_node, const char* p_name, int p_type, const void * p_obj );
            KSymbol * CreateLocalSymbol ( const AST & p_node, const String & p_name, int p_type, const void * p_obj );

            KSymbol * CreateConstSymbol ( const char* p_name, int p_type, const void * p_obj );

            struct STypeExpr * MakeTypeExpr ( const AST & p_type );

            // false - failed, error reported
            bool VectorAppend ( Vector & self, uint32_t *idx, const void *item );

            bool CreateOverload ( const KSymbol * p_name,
                                  const void * p_object,
                                  int64_t (CC * p_sort)(const void *, const void *),
                                  Vector & p_objects,
                                  Vector & p_names,
                                  uint32_t * p_id );

            bool HandleFunctionOverload ( const void * p_object, uint32_t p_version, const KSymbol * p_priorDecl, uint32_t * p_id );
            bool HandlePhysicalOverload ( const void * p_object, uint32_t p_version, const KSymbol * p_priorDecl, uint32_t * p_id );
            struct SExpression * HandlePhysicalBody ( const String & p_name,
                                                      const AST & p_schema,
                                                      const AST & p_returnType,
                                                      const AST & p_fact,
                                                      const AST & p_body,
                                                      SFunction & p_func,
                                                      bool p_makeReturn );

            bool HandleDbOverload ( const struct SDatabase *     p_db,
                                    uint32_t                     p_version,
                                    const KSymbol *              p_priorDecl,
                                    uint32_t *                   p_id );

            void AddProduction ( const AST &        p_node,
                                 Vector &           p_list,
                                 const char *       p_name,
                                 const AST_Expr &   p_expr,
                                 const AST *        p_type );

            bool FillSchemaParms ( const AST & p_parms, Vector & p_v );
            bool FillFactoryParms ( const AST & p_parms, Vector & p_v );
            bool FillArguments ( const AST & p_parms, Vector & p_v );

            const void * SelectVersion ( const AST_FQN & fqn,
                                         const struct KSymbol & p_ovl,
                                         int64_t ( CC * p_cmp ) ( const void *item, const void *n ),
                                         uint32_t * p_version = 0 ); // OUT, NULL OK

            const KSymbol * TypeSpec ( const AST & p_spec, VTypedecl & p_td );

            bool CheckForColumnCollision ( const KSymbol *sym );
            bool ScanVirtuals ( const Token :: Location & p_loc, Vector & p_byParent );

            const struct SView * GetView () const { return m_view; }

        private:
            bool Init();

            void DeclareType ( const AST_FQN& fqn, const KSymbol& super, const AST_Expr* dimension_opt );
            void DeclareTypeSet ( const AST_FQN& fqn, const BSTree& types, uint32_t typeCount );

            struct SFormParmlist * MakeFactoryParams ( const AST & );
            void AddFactoryParams ( Vector& sig, const AST & params );

            struct SFormParmlist * MakeFormalParams ( const AST & );
            void AddFormalParams ( Vector& sig, const AST & params );

            uint64_t EvalConstExpr ( const AST_Expr &expr );

            const struct KFile * OpenIncludeFile ( const Token :: Location & p_loc, const char * p_fmt, ... );

        private:
            VSchema*    m_schema;
            KSymTable   m_symtab;

            ErrorReport m_errors;
            uint32_t    m_nextContextId;

            const SView * m_view; // 0 if not inside a view declaration
        };


        template < typename T >
        T*
        ASTBuilder :: Alloc ( size_t p_size )
        {
            T * ret = static_cast < T * > ( malloc ( p_size ) ); // VSchema's tables dispose of objects with free()
            if ( ret == 0 )
            {
                ReportRc ( "malloc", RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted ) );
            }
            memset ( ret, 0, p_size );
            return ret;
        }

    }
}

#endif
