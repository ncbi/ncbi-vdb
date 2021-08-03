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

#include <memory/UniqueRawMemoryBlock.hpp>

// memset, memmove
#include <cstring>

using namespace VDB3;

/////////////// UniqueRawMemoryBlock

UniqueRawMemoryBlock :: UniqueRawMemoryBlock ( MemoryMgr p_mgr, bytes_t p_size )
:   MemoryBlockItf ( p_mgr ),
    m_size ( p_size ),
    m_ptr ( ( byte_t * ) p_mgr -> allocateUntracked ( m_size ), Deleter ( p_mgr, * this ) )
{
}

UniqueRawMemoryBlock :: ~UniqueRawMemoryBlock()
{
}

bytes_t UniqueRawMemoryBlock :: size() const
{
    return m_size;
}

void UniqueRawMemoryBlock :: fill(byte_t filler)
{
    memset( m_ptr . get(), int ( filler ), size() );
}

UniqueRawMemoryBlock
UniqueRawMemoryBlock :: clone() const
{
    UniqueRawMemoryBlock ret ( getMgr(), m_size );
    memmove ( ret . getPtr() . get(), m_ptr . get(), m_size );
    return ret;
}

void
UniqueRawMemoryBlock :: resize ( bytes_t new_size )
{
    byte_t * ptr = m_ptr . release ();
    m_ptr . reset ( ( byte_t * ) getMgr() -> reallocateUntracked ( ptr, size (), new_size ) );
    m_size = new_size;
}
