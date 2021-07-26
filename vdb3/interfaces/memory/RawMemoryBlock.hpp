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
 *  Raw memory block, a sequence of bytes. Reference-counted. Non-resizable.
 */
class RawMemoryBlock : public MemoryBlockItf
{
public:
    /**
     * Constructor
     * @param mgr instance of a memory manager that allocated this block. Will be used for deallocation.
     * @param size size of the block
    */
    RawMemoryBlock ( MemoryMgr mgr, bytes_t size );

    /**
     * Copy constructor.
     * @param that source
     */
    RawMemoryBlock( const RawMemoryBlock & that );

    virtual ~RawMemoryBlock();

public: // inherited from MemoryBlockItf
    virtual const void * ptr () const { return m_ptr . get (); };
    virtual bytes_t size() const;

public:
    /**
     * Access the underlying bytes for read/write.
     * @return underlying bytes
    */
    byte_t * data() noexcept { return m_ptr . get (); }

    /**
     * Access the underlying bytes for read.
     * @return underlying bytes
    */
    const byte_t * data() const noexcept { return m_ptr . get (); }

    /**
     * Number of references to this block.
     * @return Number of references to this block.
    */
    virtual unsigned long refcount () const noexcept { return ( unsigned long ) m_ptr . use_count(); }

    /**
     * Create a copy of the block, with reference count 1.
     * @return a copy of the block, with reference count 1.
    */
    RawMemoryBlock clone() const;

    /**
     * Fill the block with specified value.
     * @param filler vallue to fill the block with
    */
    void fill ( byte_t filler );

protected:
    /**
     * Shared pointer to the sequence of bytes
    */
    typedef std :: shared_ptr < byte_t > PtrType;

    /**
     * Read/write access to the shared pointer
     * @return the shared pointer
    */
    PtrType getPtr() { return m_ptr; }

    /**
     * Read access to the shared pointer
     * @return the shared pointer
    */
    const PtrType getPtr() const { return m_ptr; }

private:
    /**
     * Assignment - deleted
     * @param that source
     * @return *this
    */
    RawMemoryBlock & operator = ( const RawMemoryBlock & that );

    class Deleter;

    bytes_t m_size;  ///< size of the block, in bytes
    PtrType m_ptr;  ///< pointer to the underlying bytes
};

} // namespace VDB3

