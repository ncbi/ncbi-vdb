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

#include <memory/MemoryManagerItf.hpp>

namespace VDB3
{

/**
 *  Interface for a reference-counted memory block.
 */
class MemoryBlockItf
{
public:
    /**
     * Retrieve a pointer to the block's allocated memory segment. Used for memory management purposes only; to access the object use data() in the subclasses.
     * @return pointer to the block's allocated memory segment
     */
    virtual const void * ptr () const = 0;

    /**
     * Size of the memory block, in bytes.
     * @return Size of the memory block, in bytes.
     */
    virtual bytes_t size() const = 0;

    /**
     * Number of references to this block.
     * @return Number of references to this block.
    */
    virtual unsigned long refcount () const noexcept = 0 ;


protected:
    /**
     * Constructor.
     * @param p_mgr associated memory manager
     */
    MemoryBlockItf ( MemoryMgr p_mgr );

    /**
     * Copy constructor.
     * @param that source
     */
    MemoryBlockItf ( const MemoryBlockItf & that );

    virtual ~MemoryBlockItf() = 0;

    /**
     * Access to the memory manager associated with the block.
     * @return instance of memory manager that allocated this block
     */
    MemoryMgr getMgr() const { return mgr; }

private:
    /**
     * Assignment - disabled
     * @param that source
     * @return *this
     */
    MemoryBlockItf & operator = ( const MemoryBlockItf & that );

    MemoryMgr mgr; ///< the memory manager instance to be used for deallocation
};

} // namespace VDB3



