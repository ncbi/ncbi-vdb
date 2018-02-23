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

#include <klib/text.h>

#include <kfs/mmap.h>

#include "SchemaScanner.hpp"
#include "ParseTree.hpp"

using namespace ncbi::SchemaParser;
#define YYDEBUG 1
#include "schema-tokens.h"

SchemaParser :: SchemaParser ()
:   m_root ( 0 )
{
}

SchemaParser :: ~SchemaParser ()
{
    delete m_root;
}

bool
SchemaParser :: ParseString ( const char * p_input, bool p_debug )
{
    SchemaScanner s ( p_input, string_size ( p_input ), false );
    Schema_debug = p_debug;
    delete m_root;
    m_root = 0;
    return Schema_parse ( & m_root, & m_errors, & s . GetScanBlock () ) == 0;
}

bool
SchemaParser :: ParseFile ( const struct KFile * p_file, const char * p_fileName )
{
    delete m_root;
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
                ret = Schema_parse ( & m_root, & m_errors, & s . GetScanBlock () ) == 0;
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