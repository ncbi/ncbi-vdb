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

#include "SchemaParser.hpp"

#include <kfc/except.h>

#include <klib/text.h>
#include <klib/log.h>

#include <kfs/mmap.h>

#include "../vdb/schema-priv.h"

#include "SchemaScanner.hpp"
#include "ParseTree.hpp"
#include "ASTBuilder.hpp"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-grammar.hpp"

SchemaParser :: SchemaParser ()
:   m_root ( 0 )
{
}

SchemaParser :: ~SchemaParser ()
{
    ParseTree :: Destroy ( m_root );
}

bool
SchemaParser :: ParseString ( ctx_t ctx, const char * p_input, bool p_debug )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    SchemaScanner s ( p_input, string_size ( p_input ), false );
    Schema_debug = p_debug;
    ParseTree :: Destroy ( m_root );
    m_root = 0;
    return Schema_parse ( ctx, & m_root, & m_errors, & s . GetScanBlock () ) == 0;
}

bool
SchemaParser :: ParseFile ( ctx_t ctx, const struct KFile * p_file, const char * p_fileName )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    ParseTree :: Destroy ( m_root );
    m_root = 0;

    assert ( p_file != 0 );

    bool ret = false;
    const KMMap *mm;
    rc_t rc = KMMapMakeRead ( & mm, p_file );
    if ( rc == 0 )
    {
        const char *addr;
        rc = KMMapAddrRead ( mm, ( const void ** ) & addr );
        if ( rc == 0 )
        {
            size_t size;
            rc = KMMapSize ( mm, & size );
            if ( rc == 0 )
            {
                SchemaScanner s ( addr, size, false );
                s . GetScanBlock () . file_name = p_fileName == 0 ? "" : p_fileName;
                ret = Schema_parse ( ctx, & m_root, & m_errors, & s . GetScanBlock () ) == 0;
            }
        }

        KMMapRelease ( mm );
    }

    return ret;
}

ParseTree*
SchemaParser :: MoveParseTree ()
{
    ParseTree* ret = m_root;
    m_root = 0;
    return ret;
}

static
void
LogErrors ( ctx_t ctx, const ErrorReport & p_rep )
{
    FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
    uint32_t count = p_rep . GetCount ();
    for (uint32_t i = 0; i < count; ++i)
    {
        char buf [1024];
        p_rep . GetError ( i ) -> Format ( ctx, buf, sizeof ( buf )  );
        LogMsg ( klogErr, buf );
    }
}

bool
VSchemaParse_v2 ( VSchema * p_self, const char * p_text, size_t p_bytes )
{
    HYBRID_FUNC_ENTRY( rcSRA, rcSchema, rcParsing ); // entry point into the v2 schema parser
    ncbi :: SchemaParser :: SchemaParser parser;

    // bison wants its input 0-terminated
    char * zeroTerm = string_dup ( p_text, p_bytes );

    if ( ! parser . ParseString ( ctx, zeroTerm ) )
    {
        LogErrors ( ctx, parser . GetErrors () );
        free ( zeroTerm );
        return false;
    }
    free ( zeroTerm );

    ncbi :: SchemaParser :: ParseTree * parseTree = parser . MoveParseTree ();
    assert ( parseTree != 0 );

    ncbi :: SchemaParser :: ASTBuilder builder ( ctx, p_self );
    ncbi :: SchemaParser :: AST * ast = builder . Build ( ctx, * parseTree, "", false );
    AST :: Destroy ( ast );
    ParseTree :: Destroy ( parseTree );

    if ( builder . GetErrorCount() != 0)
    {
        LogErrors ( ctx, builder . GetErrors () );
        return false;
    }
    if ( FAILED () )
    {
        LogMsg ( klogErr, WHAT() );
        return false;
    }
    return true;
}
