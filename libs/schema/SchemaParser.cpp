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

#include "SchemaScanner.hpp"
#include "ParseTree.hpp"

using namespace ncbi::SchemaParser;
#include "schema-tokens.h"

extern int Schema_debug;

SchemaParser :: SchemaParser ( bool p_debug )
:   m_debug ( p_debug )
{
}

SchemaParser :: ~SchemaParser ()
{
}

bool
SchemaParser :: ParseString ( const char * input, ParseTree*& p_tree )
{
    SchemaScanner s ( input );
    Schema_debug = m_debug;
    p_tree = 0;
    return Schema_parse ( & m_root, & s . GetScanBlock () ) == 0;
}

bool
SchemaParser :: ParseFile ( const char * source_file, ParseTree*& p_tree )
{
    p_tree = 0;
    return false;
}

ParseTree*
SchemaParser :: GetRoot () const
{
    return m_root;
}

void
SchemaParser :: SetRoot ( ParseTree* p_root )
{
    m_root = p_root;
}
