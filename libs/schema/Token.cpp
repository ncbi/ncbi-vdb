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

#include "Token.hpp"

#include <klib/text.h>

using namespace ncbi :: SchemaParser;

const Token :: TokenType Token :: EndSource;

Token :: Location :: Location ( const char * p_file, uint32_t p_line, uint32_t p_column )
:   m_file ( p_file ), // no copy made
    m_line ( p_line ),
    m_column ( p_column )
{
}

Token :: Token ( const SchemaToken & p_token )
:   m_type ( p_token . type ),
    m_value ( string_dup ( p_token . value, p_token . value_len ) ),
    m_location ( p_token . file, p_token . line, p_token . column ),
    m_ws ( p_token . leading_ws )
{
}

Token :: Token ( TokenType p_type, const char * p_value )
:   m_type ( p_type ),
    m_value ( string_dup_measure ( p_value, 0 ) ),
    m_location ( "", 0, 0 ),
    m_ws ( 0 )
{
}

Token :: Token ( TokenType p_type, const char * p_value, const Location & p_loc )
:   m_type ( p_type ),
    m_value ( string_dup_measure ( p_value, 0 ) ),
    m_location ( p_loc . m_file, p_loc . m_line, p_loc . m_column ),
    m_ws ( 0 )
{
}

Token :: Token ( const Token& p_token )
:   m_type ( p_token . m_type ),
    m_value ( string_dup_measure ( p_token . m_value, 0 ) ),
    m_location ( p_token . m_location ),
    m_ws ( string_dup_measure ( p_token . m_ws, 0 ) )
{
}

Token :: ~Token ()
{
    free ( m_value );
    free ( m_ws );
}
