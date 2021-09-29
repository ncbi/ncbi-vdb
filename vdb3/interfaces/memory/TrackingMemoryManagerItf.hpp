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
#pragma once

#include <map>

#include <memory/MemoryManagerItf.hpp>

namespace VDB3
{

class TrackingMemoryManagerItf;
typedef std :: shared_ptr < TrackingMemoryManagerItf > TrackingMemoryMgr;

/**
 * A memory manager interface for tracking allocated blocks and their sizes
 */
class TrackingMemoryManagerItf : public MemoryManagerItf
{
public:
    /**
     * Return size of a previously allocated memory block
     * @param ptr pointer to the memory block
     * @return size of the memory block, in bytes
     * @exception ??? if the block is not tracked by this instance
     */
    virtual size_type getBlockSize ( const void * ptr ) const = 0;

    /**
     * Set/update size of a memory block. Will add the block if it is not present.
     * @param ptr pointer to the memory block
     * @param size memory block's size in bytes
     */
    virtual void setBlockSize ( const void * ptr, size_type size ) = 0;

protected:
    /**
     * Do memory manager specific post-processing of a newly allocated memory block
     * @param ptr pointer to the memory block
     * @param size of the block
    */
    virtual void onAllocate ( void * ptr, size_type size ) = 0;

    /**
     * Do memory manager specific pre-processing of a memory block about to be deallocated
     * @param ptr pointer to the memory block
     * @param size of the block
    */
    virtual void onDeallocate ( void * ptr, size_type size ) = 0;
};

/**
 * A memory manager that tracks allocated blocks and their sizes
 */
class TrackingMemoryManager : public TrackingMemoryManagerItf
{
public:
    /**
     * Constructor.
     * @param baseMgr a memory manager to handle allocation/deallocation
     */
    TrackingMemoryManager( MemoryMgr baseMgr );
    TrackingMemoryManager();
    virtual ~TrackingMemoryManager();

public: // inherited from MemoryManagerItf
    // VDB3-facing API
    // calls to these methods are forwarded to the base manager, block sizes are not tracked
    virtual void * allocateUntracked ( bytes_t size );
    virtual void * reallocateUntracked ( void * ptr, bytes_t cur_size, bytes_t new_size );
    virtual void deallocateUntracked ( void * ptr, bytes_t size ) noexcept;

public: // inherited from MemoryManagerItf
    // STL facing API
    virtual pointer allocate ( size_type size );
    virtual pointer reallocate ( void * ptr, size_type new_size);
    virtual void deallocate ( void * ptr, size_type size ) noexcept;

public: // inherited from TrackingMemoryManagerItf
    virtual size_type getBlockSize ( const void * ptr ) const;
    virtual void setBlockSize ( const void * ptr, size_type size ); // adds if necessary

protected:
    virtual void onAllocate ( void * ptr, size_type size ) {}
    virtual void onDeallocate ( void * ptr, size_type size ) {}

private:
    /**
     * A structure to keep track of all memory blocks with their sizes
     */
    typedef std::map < const_pointer,
                       size_type,
                       std :: less < const_pointer >,
                       MemoryManagerItf :: allocator < std :: pair < const const_pointer, size_type > >
            >
            Blocks;

private:
    MemoryMgr m_baseMgr; ///< memory manager doing the allocation.
    Blocks m_blocks; ///< all tracked blocks
};

/**
 * A memory manager that relies on an instance of TrackingMemoryManagerItf to keep track of allocated blocks and their sizes.
 * Can be used to chain several TrackingMemoryManagerItf's together (with a TrackingMemoryManager at the bottom of the chain)
 */
class TrackingBypassMemoryManager : public TrackingMemoryManagerItf
{
public:
    /**
     * Constructor.
     * @param baseMgr a memory manager to handle allocation/deallocation and memory block tracking
     */
    TrackingBypassMemoryManager ( TrackingMemoryMgr baseMgr );
    TrackingBypassMemoryManager ();
    virtual ~TrackingBypassMemoryManager ();

public: // inherited from MemoryManagerItf
    // VDB3-facing API
    // calls to these methods are forwarded to the base manager, block sizes are not tracked
    virtual void * allocateUntracked ( bytes_t size );
    virtual void * reallocateUntracked ( void * ptr, bytes_t cur_size, bytes_t new_size );
    virtual void deallocateUntracked ( void * ptr, bytes_t size ) noexcept;

public: // inherited from MemoryManagerItf
    // STL facing API
    virtual pointer allocate ( size_type size );
    virtual pointer reallocate ( pointer ptr, size_type new_size) ;
    virtual void deallocate ( pointer ptr, size_type size ) noexcept;

public: // inherited from TrackingMemoryManagerItf
    virtual size_type getBlockSize ( const void * ptr ) const;
    virtual void setBlockSize ( const void * ptr, size_type size ); // adds if necessary

protected:
    virtual void onAllocate ( void * ptr, size_type size ) {}
    virtual void onDeallocate ( void * ptr, size_type size ) {}

protected:
    /**
     * Access the base memory manager
     * @return the base memory manager
     */
    TrackingMemoryMgr & baseMgr () { return m_baseMgr; }

private:
    TrackingMemoryMgr  m_baseMgr; ///< memory manager doing the tracking/allocation.
};

} // namespace VDB3

