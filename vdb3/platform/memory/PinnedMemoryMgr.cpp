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

#include <memory/PinnedMemoryMgr.hpp>

#include <sys/mman.h>
#include <cassert>

#include <memory/PrimordialMemoryMgr.hpp>

using namespace VDB3;

PinnedMemoryMgr :: MemoryLockerItf :: ~MemoryLockerItf ()
{
}

/**
* Default memory locker. Uses mlock/munlock.
*/
class PosixMemoryPinner : public PinnedMemoryMgr :: MemoryLockerItf
{
public:
    PosixMemoryPinner() {}
    virtual ~PosixMemoryPinner () {}
    /**
     * Locks memory in the address range starting at ptr and continuing for bytes.
     * @param ptr pointer to the start of the address range
     * @param bytes size of the address range
     * @exception ???
     */
    virtual void lock( MemoryManagerItf :: pointer ptr, bytes_t bytes )
    {
        int res = mlock ( ptr, bytes );
        if ( res != 0 )
        {
            throw std :: logic_error ( "mlock() failed" );
        }
    }
    /**
     * Unocks memory in the address range starting at ptr and continuing for bytes.
     * @param ptr pointer to the start of the address range
     * @param bytes size of the address range
     * @exception ???
     */
    virtual void unlock( MemoryManagerItf :: pointer ptr, bytes_t bytes )
    {
        int res = munlock ( ptr, bytes );
        if ( res != 0 )
        {
            throw std :: logic_error ( "munlock() failed" );
        }
    }
};

PosixMemoryPinner linuxLocker; ///< the default memory locker

PinnedMemoryMgr :: PinnedMemoryMgr( TrackingMemoryMgr base_mgr, MemoryLockerItf * locker )
:   TrackingBypassMemoryManager ( base_mgr ),
    m_locker ( locker == nullptr ? linuxLocker : * locker)
{
}

PinnedMemoryMgr :: PinnedMemoryMgr( MemoryLockerItf * locker )
:   m_locker ( locker == nullptr ? linuxLocker : * locker)
{
}

PinnedMemoryMgr :: ~PinnedMemoryMgr()
{
}

void
PinnedMemoryMgr :: onAllocate ( void * ptr, size_type size )
{
    if ( ptr != nullptr )
    {
        m_locker . lock( ptr, size );
    }
}

void
PinnedMemoryMgr :: onDeallocate ( void * ptr, size_type size )
{
    if ( ptr != nullptr )
    {
        m_locker . unlock( ptr, size );
    }
}

void *
PinnedMemoryMgr :: reallocateUntracked( void * old_ptr, bytes_t old_size, bytes_t new_size )
{
    if ( old_ptr == nullptr )
    {
        return allocateUntracked ( new_size );
    }
    if ( new_size == 0 )
    {
        deallocateUntracked ( old_ptr, old_size );
        return nullptr;
    }

    pointer new_ptr =  baseMgr () -> reallocateUntracked( old_ptr, old_size, new_size );
    if ( old_ptr != new_ptr )
    {   // unpin the old block
        onDeallocate ( old_ptr, old_size );
    }

    if ( new_ptr != nullptr )
    {   // pin the new block
        onAllocate ( new_ptr, new_size );
    }

    return new_ptr;
}

PinnedMemoryMgr :: pointer
PinnedMemoryMgr :: reallocate ( pointer old_ptr, size_type new_size )
{
    if ( old_ptr == nullptr )
    {
        return allocate ( new_size );
    }

    size_t old_size = baseMgr () -> getBlockSize( old_ptr ); // will throw if bad block
    if ( new_size == 0 )
    {
        deallocate ( old_ptr, old_size );
        return nullptr;
    }

    pointer new_ptr =  baseMgr () -> reallocate( old_ptr, new_size );

    onDeallocate ( old_ptr, old_size );
    onAllocate ( new_ptr, new_size );

    return new_ptr;
}

