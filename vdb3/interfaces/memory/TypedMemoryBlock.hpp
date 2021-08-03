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

#include <memory/MemoryBlockItf.hpp>

namespace VDB3
{

/**
 * A typed, reference-counted memory block.
 * TODO: move method implementations to an adjunct header
 */
template <typename T> class TypedMemoryBlock : public MemoryBlockItf
{
public:
    /**
     * Constructor
     * @param p_mgr instance of a memory manager to allocate this block. Will also be used for deallocation.
     * @param p_args arguments to be passed to T's constructor
    */
    template<typename... Args> TypedMemoryBlock( MemoryMgr p_mgr, const Args&& ... p_args )
    :   MemoryBlockItf ( p_mgr ),
        m_ptr ( ( T * ) getMgr() -> allocateUntracked ( sizeof ( T ) ), Deleter ( getMgr() ) )
    {
        new ( m_ptr.get() ) T ( p_args ...);
    }

    /**
     * Copy constructor.
     * @param that source
     */
    TypedMemoryBlock( const TypedMemoryBlock & that )
    :   MemoryBlockItf ( that . getMgr() ),
        m_ptr ( that . m_ptr )
    {
    }

    virtual ~TypedMemoryBlock()
    {
    }

public: // inherited from MemoryBlockItf
    virtual const void * ptr () const { return m_ptr . get (); };
    virtual bytes_t size() const { return sizeof ( T ); }

public:

    /**
     * Conversion to the base type
     * @return underlying value of T
     */
    operator T& () { return * m_ptr . get (); }

    /**
     * Conversion to the base type, read-only
     * @return underlying value of T
     */
    operator const T& () const  { return * m_ptr . get (); }

    /**
     * Read/write access to the typed value
     * @return underlying value of T
     */
    T& data() noexcept { return * m_ptr . get (); }

    /**
     * Read access to the typed value
     * @return underlying value of T
     */
    const T& data() const noexcept { return * m_ptr . get (); }

    /**
     * Number of references to this block.
     * @return Number of references to this block.
    */
    virtual unsigned long refcount () const noexcept { return ( unsigned long ) m_ptr . use_count(); }

    /**
     * Copy-construct a new typed value with reference count of 1.
     * T is required to define a copy constructor
     * @return a copy with reference count of 1.
     * @exception TODO: ??? if failed
     */
    TypedMemoryBlock clone() const
    {
        return TypedMemoryBlock ( getMgr(), std::forward<const T>( data() ) );
    };

public:
    /**
    *  Deleter class, passed to std::shared_ptr.
    */
    class Deleter
    {
    public:
        /**
         * Constructor.
         * @param p_mgr instance of memory manager to be used for deallocation
         */
        Deleter ( MemoryMgr p_mgr ) : m_mgr ( p_mgr ) {}

        /**
         * Destroy the object and deallocate its memory block using its associated memory manager.
         * @param p the block to be deallocated
         */
        void operator() ( T * p ) const
        {
            if ( p != nullptr )
            {
                p -> ~T();
            }
            m_mgr -> deallocateUntracked ( p, sizeof ( T ) );
        }

    private:
        MemoryMgr m_mgr; ///< the memory manager instance to be used for deallocation
    };

private:
    /**
     * Assignment - deleted.
     * @param that source
     * @return *this
     */
    TypedMemoryBlock & operator = ( const TypedMemoryBlock & that );

    std :: shared_ptr < T > m_ptr; ///< pointer to the shared block
};

} // namespace VDB3

