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

#include <memory/RawMemoryBlock.hpp>

// memset, memmove
#include <cstring>

using namespace VDB3;

/////////////// RawMemoryBlock :: Deleter

/**
*  Deleter class, used to wrap MemoryBlockItf in "allocator" classes passed to STL.
*/
class RawMemoryBlock :: Deleter
{
public:
    /**
     * Constructor.
     * @param p_mgr instance of memory manager to be used for deallocation
     * @param p_size size of the block, in bytes
     */
    Deleter ( MemoryMgr p_mgr, bytes_t p_size ) : m_mgr ( p_mgr ), m_size ( p_size ) {}

    /**
     * Deallocate a memory block using its associated memory manager.
     * @param p the block to be deallocated
     */
    void operator() ( byte_t * p ) const
    {
        m_mgr -> deallocateUntracked ( p, m_size );
    }

private:
    MemoryMgr m_mgr; ///< the memory manager instance to be used for deallocation
    size_t m_size; ///< size of the block, in bytes
};

/////////////// RawMemoryBlock

RawMemoryBlock :: RawMemoryBlock ( MemoryMgr p_mgr, bytes_t p_size )
:   MemoryBlockItf ( p_mgr ),
    m_size ( p_size ),
    m_ptr ( ( byte_t * ) p_mgr -> allocateUntracked ( m_size ), Deleter( p_mgr, m_size ) )
{
}

RawMemoryBlock :: RawMemoryBlock( const RawMemoryBlock & that )
:   MemoryBlockItf ( that . getMgr() ),
    m_size ( that . size () ),
    m_ptr ( that . getPtr () )
{
}

RawMemoryBlock :: ~RawMemoryBlock()
{
}

bytes_t RawMemoryBlock :: size() const
{
    return m_size;
}

void RawMemoryBlock :: fill(byte_t filler)
{
    memset( m_ptr . get(), int ( filler ), size() );
}

RawMemoryBlock RawMemoryBlock :: clone() const
{
    RawMemoryBlock ret ( getMgr(), m_size );
    memmove ( ret . getPtr() . get(), m_ptr . get(), m_size );
    return ret;
}
