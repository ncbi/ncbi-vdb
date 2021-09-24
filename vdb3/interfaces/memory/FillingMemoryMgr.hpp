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

#include <memory/TrackingMemoryManagerItf.hpp>

namespace VDB3
{

/**
 *  Memory manager making sure that all new blocks abd deallocated are filled with specific byte values.
 */
class FillingMemoryMgr : public TrackingBypassMemoryManager
{
public:
    /**
     * Default value to fill newly allocated blocks with
     */
    static const byte_t DefaultFiller = byte_t ( 0 );
    /**
     * Default value to fill dellocated blocks with
     */
    static const byte_t DefaultTrash  = byte_t ( 0 );

public:
    /**
     * Constructor
     * @param base_mgr optional pointer to a memory manager to handle allocation/deallocation and keep track of block sizes. If nullptr, a default manager will be used.
     * @param fill_byte immediately after allocation, the block will be filled with this value
     * @param trash_byte immediately before dealocation, the block will be filled with this value
    */
    FillingMemoryMgr( TrackingMemoryMgr base_mgr, byte_t fill_byte = DefaultFiller, byte_t trash_byte = DefaultTrash );

    /**
     * Constructor. Uses a default memory manager to handle allocation/deallocation and keep track of block sizes.
     * @param fill_byte immediately after allocation, the block will be filled with this value
     * @param trash_byte immediately before dealocation, the block will be filled with this value
    */
    FillingMemoryMgr( byte_t fill_byte = DefaultFiller, byte_t trash_byte = DefaultTrash );

    virtual ~FillingMemoryMgr();

    /**
     * Byte value used to fill newly allocated blocks
     * @return Byte value used to fill newly allocated blocks
    */
    byte_t fillByte () const { return m_fillByte; }

    /**
     * Byte value used to fill deallocated blocks
     * @return Byte value used to fill deallocated blocks
    */
    byte_t trashByte () const { return m_trashByte; }

public: // inherited from MemoryManagerItf

    virtual void * reallocateUntracked ( void * block, bytes_t cur_size, bytes_t new_size );

    /**
     * Change the size of a block, possibly reallocating it. Any extra bytes will be filled with fill_byte.
     * If the block shrinks, the trailing bytes will be filled with trash_byte. If the block gets moved, its
     * old location will be filled with trash_bytes before being released by the underlying memory manager
     * (which can do its own thing on top)
     *
     * @param ptr see MemoryManagerItf
     * @param new_size see MemoryManagerItf
     * @return see MemoryManagerItf
     * @exception see MemoryManagerItf
     */
    virtual pointer reallocate ( pointer ptr, size_type new_size );

protected:
    virtual void onAllocate ( void * ptr, size_type bytes );
    virtual void onDeallocate ( void * ptr, size_type bytes );

private:
    byte_t m_fillByte;    ///< byte value to fill new blocks with
    byte_t m_trashByte;   ///< byte value to fill deallocated blocks with
};

} // namespace VDB3

