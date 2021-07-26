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

#include <memory/SharedMemoryMgr.hpp>

#include <sys/mman.h>
#include <cassert>
#include <cstring>

#include <memory/PrimordialMemoryMgr.hpp>
#include <memory/MemoryBlockItf.hpp>

using namespace std;

namespace VDB3
{

/**
* Helper class implementing mapping/unmapping of shared memory blocks for SharedMemoryMgr
*/
class SharedMemoryMgr :: SharedMemoryMgr_Internal : public MemoryManagerItf
{

public: // inherited from MemoryManagerItf
    virtual void * allocateUntracked ( bytes_t bytes )
    {
        return allocate ( size_type ( bytes ) );
    }

    virtual void * reallocateUntracked ( void * block, bytes_t cur_size, bytes_t new_size )
    {
        if ( block == nullptr )
        {
            return allocate ( new_size );
        }

        if ( cur_size == 0 )
        {
            deallocate ( block, cur_size );
            return nullptr;
        }

        // always create and copy
        void * ret = allocate ( new_size );
        memmove ( ret, block, std :: min ( cur_size, new_size ) );
        deallocate ( block, cur_size );

        return ret;
    }

    virtual void deallocateUntracked ( void * block, bytes_t size ) noexcept
    {
        deallocate ( block, size );
    }

public:
    virtual pointer allocate ( size_type bytes )
    {
        if ( bytes == 0 )
        {
            return nullptr;
        }
        //TODO: control read/write
        pointer ret = mmap ( nullptr, bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0 );
        if ( ret == MAP_FAILED )
        {
            throw std :: bad_alloc (); //TODO: use a VDB3 exception when implemented
        }
        return ret;
    }

    /**
     * Memory map a portion of file
     * @param bytes length of the portion (currently, from the beginning of the file)
     * @param fd file descriptor obtained by open()
     * @param canWrite true if to be made updateable
     * @return pointer to the beginning of the mapped block
     * @exception ??? if mapping failed
     */
    pointer allocate ( size_type bytes, int fd, bool canWrite ) //TODO: specify offset (a multiple of the page size as returned by sysconf(_SC_PAGE_SIZE).)
    {
        if ( bytes == 0 )
        {
            return nullptr;
        }

        int prot = PROT_READ;
        if ( canWrite )
        {
            prot |= PROT_WRITE;
        }
        pointer ret = mmap ( nullptr, bytes, prot, MAP_SHARED, fd, 0 );
        if ( ret == MAP_FAILED )
        {
            throw std :: bad_alloc (); //TODO: use a VDB3 exception when implemented
        }
        return ret;
    }

    virtual pointer reallocate ( pointer ptr, size_type new_size )
    {   // reallocation is dealt with in SharedMemoryMgr :: reallocate
        // since we need the old block size which is not accessible here
        throw std :: logic_error ( "SharedMemoryMgr_Internal::reallocate() called" );
    }

    virtual void deallocate ( pointer ptr, size_type bytes ) noexcept
    {
        munmap( ptr, bytes );
    }
};

//TODO: use allocate_shared< SharedMemoryMgr :: SharedMemoryMgr_Internal, MemoryManagerItf> ( pm )
// where pm is retrieved from ResourceManager
SharedMemoryMgr :: SharedMemoryMgr()
: TrackingMemoryManager ( make_shared < SharedMemoryMgr :: SharedMemoryMgr_Internal > () )
{
}

SharedMemoryMgr :: ~SharedMemoryMgr()
{
}

SharedMemoryMgr :: pointer
SharedMemoryMgr :: reallocate ( pointer old_ptr, size_type new_size )
{
    if ( old_ptr == nullptr )
    {
        return allocate ( new_size );
    }

    // need the old block size; that is why we deal with reallocation here, not in SharedMemoryMgr_Internal
    size_type old_size = getBlockSize ( old_ptr );

    if ( new_size == 0 )
    {
        deallocate ( old_ptr, old_size );
        return nullptr;
    }

    // always create and copy
    pointer ret = allocate ( new_size ); // will record the new block's size
    memmove ( ret, old_ptr, std :: min ( old_size, new_size ) );
    deallocate ( old_ptr, old_size );

    return ret;
}

// SharedMemoryMgr :: pointer
// SharedMemoryMgr :: allocate ( size_type bytes, int fd, bool canWrite, off_t offset, bool shared )
// {   // read/write must not conflict with the open mode of the file
//     pointer ret = getBaseMgr() . allocate ( bytes, fd, canWrite );
//     if ( ret != nullptr )
//     {
//         setBlockSize ( ret, bytes );
//     }
//     return ret;
// }

}