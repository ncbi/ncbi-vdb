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

#include <klib/text.h>

using namespace ncbi::SchemaParser;

static const uint32_t BlockSize = 1024; //TODO: pick a good initial size

ParseTree :: ParseTree ( const SchemaToken& p_token )
: m_token ( p_token )
{
    VectorInit ( & m_children, 0, BlockSize );
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
