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

#include <memory/TrackingMemoryManagerItf.hpp>

#include <cassert>
#include <stdexcept>

#include <memory/PrimordialMemoryMgr.hpp>

using namespace std;
using namespace VDB3;

static MemoryMgr primMgr = make_shared < PrimordialMemoryMgr > ();

//////// TrackingMemoryManager

TrackingMemoryManager :: TrackingMemoryManager( MemoryMgr baseMgr )
:   m_baseMgr( baseMgr ),
    m_blocks( * m_baseMgr )
{
}

TrackingMemoryManager :: TrackingMemoryManager ()
:   m_baseMgr( primMgr ),
    m_blocks( * m_baseMgr )
{
}

TrackingMemoryManager :: ~TrackingMemoryManager ()
{
}

void *
TrackingMemoryManager :: allocateUntracked( bytes_t size )
{
    void * ret = m_baseMgr -> allocateUntracked ( size );
    onAllocate ( ret, size );
    return ret;
}

void *
TrackingMemoryManager :: reallocateUntracked( void * ptr, bytes_t cur_size, bytes_t new_size )
{
    return m_baseMgr -> reallocateUntracked( ptr, cur_size, new_size );
}

void
TrackingMemoryManager :: deallocateUntracked( void * ptr, bytes_t size ) noexcept
{
    onDeallocate ( ptr, size );
    m_baseMgr -> deallocateUntracked( ptr, size );
}

TrackingMemoryManager :: pointer
TrackingMemoryManager :: allocate( size_type size )
{
    void * ret = m_baseMgr -> allocate ( size );
    if ( ret != nullptr )
    {
        m_blocks [ ret ] = size;
    }
    onAllocate ( ret, size );
    return ret;
}

TrackingMemoryManager :: pointer
TrackingMemoryManager :: reallocate( pointer ptr, size_type new_size )
{
    if ( ptr == nullptr )
    {
        return reallocateUntracked( ptr, 0, new_size );
    }
    if ( new_size == 0 )
    {
        deallocate ( ptr, 0 );
        return nullptr;
    }

    Blocks::const_iterator it = m_blocks . end();
    it = m_blocks . find( ptr );
    if ( it == m_blocks . end () )
    {
        throw std :: logic_error( "TrackingMemoryManager :: getBlockSize () called with an unknown block" ); //TODO: replace with a VDB3 exception
    }

    pointer ret = reallocateUntracked( ptr, it -> second, new_size );
    if ( ret != nullptr )
    {
        if ( ret != ptr )
        {
            m_blocks . erase( it );
        }
        m_blocks [ ret ] = new_size;
    }
    else
    {
        m_blocks . erase( ptr );
    }

    return ret;
}

void
TrackingMemoryManager :: deallocate( pointer ptr, size_type size ) noexcept
{
    try
    {   // getBlockSize() throws on an untracked block
        onDeallocate ( ptr, getBlockSize ( ptr ) );
    }
    catch (...)
    {
        //TODO: log
    }

    auto it = m_blocks . find(ptr);
    if ( it != m_blocks . end () )
    {
        m_baseMgr -> deallocateUntracked( ptr, it -> second );
        m_blocks . erase( it );
    }
}

TrackingMemoryManager :: size_type
TrackingMemoryManager :: getBlockSize( const void * ptr ) const
{
    auto it = m_blocks . find(ptr);
    if ( it == m_blocks . end () )
    {
        throw std :: logic_error( "TrackingMemoryManager :: getBlockSize () called with an unknown block" ); //TODO: replace with a VDB3 exception
    }
    else
    {
        return it -> second;
    }
}

void
TrackingMemoryManager :: setBlockSize( const void * ptr, size_type size )
{
    m_blocks [ ptr ] = size;
}

//////// TrackingBypassMemoryManager

static TrackingMemoryMgr trackingMgr = make_shared < TrackingMemoryManager > ();

TrackingBypassMemoryManager :: TrackingBypassMemoryManager( TrackingMemoryMgr baseMgr )
:   m_baseMgr( baseMgr )
{
}

TrackingBypassMemoryManager :: TrackingBypassMemoryManager ()
:   m_baseMgr( trackingMgr )
{
}

TrackingBypassMemoryManager :: ~TrackingBypassMemoryManager ()
{
}

void *
TrackingBypassMemoryManager :: allocateUntracked( bytes_t size )
{
    void * ret = m_baseMgr -> allocateUntracked ( size );
    onAllocate ( ret, size );
    return ret;
}

void *
TrackingBypassMemoryManager :: reallocateUntracked( void * ptr, bytes_t cur_size, bytes_t new_size )
{
    return m_baseMgr -> reallocateUntracked( ptr, cur_size, new_size );
}

void
TrackingBypassMemoryManager :: deallocateUntracked( void * ptr, bytes_t size ) noexcept
{
    onDeallocate ( ptr, size );
    m_baseMgr -> deallocateUntracked( ptr, size );
}

TrackingBypassMemoryManager :: pointer
TrackingBypassMemoryManager :: allocate( size_type size )
{
    void * ret = m_baseMgr -> allocate ( size );
    onAllocate ( ret, size );
    return ret;
}

TrackingBypassMemoryManager :: pointer
TrackingBypassMemoryManager :: reallocate( pointer ptr, size_type new_size )
{
    return m_baseMgr -> reallocate( ptr, new_size );
}

void
TrackingBypassMemoryManager :: deallocate( pointer ptr, size_type size ) noexcept
{
    try
    {   // getBlockSize() throws on an untracked block
        onDeallocate ( ptr, getBlockSize ( ptr ) );
    }
    catch (...)
    {
        //TODO: log
    }
    m_baseMgr -> deallocate( ptr, size );
}

TrackingBypassMemoryManager :: size_type
TrackingBypassMemoryManager :: getBlockSize( const void * ptr ) const
{
    return m_baseMgr -> getBlockSize( ptr );
}

void
TrackingBypassMemoryManager :: setBlockSize( const void * ptr, size_type size )
{
    m_baseMgr -> setBlockSize( ptr, size );
}


