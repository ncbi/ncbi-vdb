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

#include <memory>

namespace VDB3
{

enum class byte : unsigned char {} ;    //  std::byte in C++17
typedef byte    byte_t;

typedef size_t  bytes_t;

class MemoryBlockItf;
class MemoryManagerItf;

typedef std :: shared_ptr < MemoryManagerItf > MemoryMgr;

/**
 *  Interface for a memory manager, providing allocation/deallocation for reference counted memory blocks.
 *
 *  Provides declarations that satisfy STL requirements for an Allocator:
 *      typedef size_type;
 *      typedef pointer;
 *      typedef const_pointer;
 *      pointer allocate ( size_type bytes );
 *      void deallocate ( pointer ptr, size_type bytes );
 *  Other requirements (e.g. copy construction) are to be satisfied by sub-classes.
 *
 *  TODO: alignment issues.
 */
class MemoryManagerItf
{
public:
    virtual ~MemoryManagerItf() = 0;

public: // VDB-facing API

    /* These 3 methods operate on assumption that the caller, not the manager, keeps track of the logical size of the memory blocks (the physical size is determined by the manager) */

    /**
     * Allocate a block of given size
     *
     * @param size requested block size in bytes. Can be 0.
     * @return handle to the allocated block
     * @exception TODO: ??? if insufficient memory
     */
    virtual void * allocateUntracked ( bytes_t size ) = 0;

    /**
     * Change the size of a block, possibly reallocating it.
     *
     * @param ptr handle to the block being reallocated. The pointer must have been returned by allocateUntracked() on the same instance of MemoryManagerItf and not previously deallocated.
     * @param cur_size the current size of the block pointed to by ptr, in bytes.
     * @param new_size requested new block size in bytes. Can be 0, resulting in deallocaiton of the block.
     * @return handle to the reallocated block. May be different from the original block. If the block has been moved, the area pointed to by the original will be deallocated. ? if the specified size was 0. The contents of the original block are bitwise copied over to the new block.
     * @exception TODO: ??? if insufficient memory. The original block is left untouched; it is not freed or moved.
     */
    virtual void * reallocateUntracked ( void * ptr, bytes_t cur_size, bytes_t new_size ) = 0;

    /**
     * Release a block.
     *
     * @param ptr pointer to the block being deallocated. The pointer must have been returned by allocateUntracked() or reallocateUntracked() on the same instance of MemoryManagerItf and not previously deallocated. Can be nullptr, which is ignored.
     * @param size block size in bytes. Must match the value previously passed to allocate.
     */
    virtual void deallocateUntracked ( void * ptr, bytes_t size ) noexcept = 0;

public: // STL-facing API
    /**
     * Size of a memory block, in bytes.
     */
    typedef size_t size_type;

    /**
     * Address of a modifiable memory block.
     */
    typedef void * pointer;

    /**
     * Address of a non-modifiable memory block.
     */
    typedef const void * const_pointer;

    /**
     * For use with STL containers
     */
    template < typename T > class allocator;

public:
    /**
     * Allocate a block of given size
     *
     * @param size requested block size in bytes. Can be 0.
     * @return pointer to the allocated block. nullptr if the specified size was 0
     * @exception TODO: ??? if insufficient memory
     */
    virtual pointer allocate ( size_type size ) = 0;

    /**
     * Change the size of a block, possibly reallocating it.
     *
     * @param ptr pointer to the block being reallocated. Can be nullptr, in which case a new block of size 'bytes' will be allocated. The pointer must have been returned by the same instance of MemoryManagerItf and not previously deallocated.
     * @param new_size requested new block size in bytes. Can be 0, resulting in deallocaiton of the block.
     * @return pointer to the reallocated block. May be different from ptr. If the block has been moved, the area pointed to by ptr will be deallocated. nullptr if the specified size was 0. The contents of the original block are bitwise copied over to the new block.
     * @exception TODO: ??? if insufficient memory. The original block is left untouched; it is not freed or moved.
     */
    virtual pointer reallocate ( pointer ptr, size_type new_size ) = 0;

    /**
     * Deallocate a block.
     *
     * @param ptr pointer to the block being deallocated. The pointer must have been returned by the same instance of MemoryManagerItf and not previously deallocated. Can be nullptr, which is ignored.
     * @param size block size in bytes. Must match the value previously passed to allocate.
     */
    virtual void deallocate ( pointer ptr, size_type size ) noexcept = 0;
};

/**
 * STL adaptor class for using MemoryManagerItf as a custom allocation with STL containers.
 */
template < typename T > class MemoryManagerItf :: allocator
{
public:
    typedef T value_type;   ///< Element type
    typedef T * pointer;    ///< Rpointer to element
    typedef T & reference;  ///< Reference to element
    typedef const T * const_pointer;    ///< Pointer to constant element
    typedef const T & const_reference;  ///< Reference to constant element
    typedef MemoryManagerItf::size_type size_type;  ///< Quantities of elements
    typedef ptrdiff_t difference_type;  ///< Difference between two pointers

public:
    /**
     * Constructor
     * @param p_mgr Memory manager instance to forward calls to
     */
    allocator( MemoryManagerItf & p_mgr )
    : m_mgr ( p_mgr )
    {
    }
    /**
     * Copy constructor
     * @param that source object.
     */
    template < typename X > allocator( const allocator < X > & that )
    : m_mgr ( that . get_mgr() )
    {
    }
    ~allocator()
    {
    }

    /**
     * Returns the address of x.
     *
     * @brief Return address
     * @param x Reference to object
     * @return A pointer to the object
     */
    pointer address ( reference x ) const noexcept { return & x; }
    /**
     * Returns the address of x.
     *
     * @brief Return address
     * @param x Reference to object
     * @return A pointer to the object
     */
    const_pointer address ( const_reference x ) const noexcept { return &x; }

    /**
     * Attempts to allocate a block of storage with a size large enough to contain n elements of member type value_type
     * (an alias of the allocator's template parameter), and returns a pointer to the first element.
     * The storage is aligned appropriately for objects of type value_type, but they are not constructed.
     *
     * @brief Allocate block of storage
     * @param n Number of elements (each of size sizeof(value_type)) to be allocated.
     * @param hint Either 0 or a value previously obtained by another call to allocate and not yet freed with deallocate.
     *  When it is not 0, this value may be used as a hint to improve performance by allocating the new block near the
     *  one specified. The address of an adjacent element is often a good choice.
     * @return A pointer to the initial element in the block of storage.
     */
    pointer allocate ( size_type n, const_pointer hint = 0 )
    {
        return reinterpret_cast < pointer > ( m_mgr . allocate ( n * sizeof ( T ) ) );
    }
    /**
     * Releases a block of storage previously allocated with member allocate and not yet released.
     *  The elements in the array are not destroyed by a call to this member function.
     *
     * @brief Release block of storage
     * @param p Pointer to a block of storage previously allocated with allocator::allocate.
     * @param n Number of elements allocated on the call to allocator::allocate for this block of storage.
     */
    void deallocate ( pointer p, size_type n )
    {
        m_mgr . deallocate ( p, n * sizeof ( T ) );
    }

    //TODO: do we need rebind() ?

    /**
     * Access to the instance of Memory Manager associated with this allocator.
     * @return instance of Memory Manager associated with this allocator
     */
    MemoryManagerItf & get_mgr() const { return m_mgr; }

private:
    MemoryManagerItf & m_mgr; ///< Memory manager instance to forward calls to
};

} // namespace VDB3

