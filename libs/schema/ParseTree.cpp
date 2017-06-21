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

#include "ParseTree.hpp"

#include <stdexcept>
#include <iostream>

#include <klib/text.h>
#include <klib/printf.h>

using namespace std;
using namespace ncbi::SchemaParser;

static const uint32_t ChildrenBlockSize = 1024; //TODO: pick a good initial size

static
void
ThrowRc ( const char* p_msg, rc_t p_rc )
{
    char msg[1024];
    string_printf ( msg, sizeof msg, NULL, "%s, rc = %R", p_msg, p_rc );
    //INTERNAL_ERROR ( xcUnexpected, "VectorRemove: rc = %R", rc );
    throw logic_error ( msg );
}

// ParseTree

ParseTree :: ParseTree ( const Token& p_token )
:   m_token ( p_token ),
    m_location ( & m_token . GetLocation () )
{
    VectorInit ( & m_children, 0, ChildrenBlockSize );
}

void DestroyChild ( void * item, void * )
{
    delete ( ParseTree * ) item;
}

ParseTree :: ~ParseTree ()
{
    VectorWhack ( & m_children, DestroyChild, NULL );
}

void
ParseTree :: AddChild ( ParseTree * p_node )
{
    assert ( m_location != 0 );
    assert ( p_node != 0 );
    if ( m_location -> m_line == 0 )
    {   // assume p_node's location will not change in the future
        m_location = & p_node -> GetLocation ();
    }
    VectorSet ( & m_children, VectorLength ( & m_children ), p_node );
}

uint32_t
ParseTree :: ChildrenCount () const
{
    return VectorLength ( & m_children );
}

const ParseTree*
ParseTree :: GetChild ( uint32_t p_idx ) const
{
    return ( const ParseTree * ) VectorGet ( & m_children, p_idx );
}

ParseTree*
ParseTree :: GetChild ( uint32_t p_idx )
{
    return ( ParseTree * ) VectorGet ( & m_children, p_idx );
}

void
ParseTree :: MoveChildren ( ParseTree& p_source )
{
    VectorWhack ( & m_children, DestroyChild, NULL );
    VectorCopy ( & p_source . m_children, & m_children );
    VectorWhack ( & p_source . m_children, NULL, NULL );
}

// ParseTreeScanner

static const uint32_t StackBlockSize    = 1024; //TODO: pick a good initial size

ParseTreeScanner :: ParseTreeScanner ( const ParseTree& p_root, const char * p_source )
: m_source ( string_dup ( p_source, string_size (p_source ) ) )
{
    VectorInit ( & m_stack, 0, StackBlockSize );
    PushNode ( & p_root );
}

ParseTreeScanner :: ~ParseTreeScanner ()
{
    free ( m_source );
    VectorWhack ( & m_stack, NULL, NULL );
}

static const ParseTree ChildrenOpen  ( Token ( '(' ) );
static const ParseTree ChildrenClose ( Token ( ')' ) );

void
ParseTreeScanner :: PushNode ( const ParseTree* p_node )
{
//cout << "pushing " << p_node -> GetToken () . GetType () << endl;
    VectorAppend ( & m_stack, NULL, p_node );
}

Token :: TokenType
ParseTreeScanner :: NextToken ( const Token*& p_token )
{
    while ( VectorLength ( & m_stack ) != 0 )
    {
        const ParseTree* node;
        rc_t rc = VectorRemove ( & m_stack, VectorLength ( & m_stack ) - 1, ( void ** ) & node );
        if ( rc != 0 )
        {
            ThrowRc ( "VectorRemove", rc );
        }

        assert ( node != 0 );
//cout << "popped " << node -> GetToken () . GetType () << endl;

        uint32_t count = node -> ChildrenCount ();
        if ( count > 0 )
        {   // push '(' children ')' in reverse order
            PushNode ( & ChildrenClose );

            for ( uint32_t i = count; i > 0; --i )
            {
                PushNode ( node -> GetChild ( i - 1 ) );
            }

            PushNode ( & ChildrenOpen );
        }

        p_token = & node -> GetToken ();
        return p_token -> GetType ();
    }
    return Token :: EndSource;
}
