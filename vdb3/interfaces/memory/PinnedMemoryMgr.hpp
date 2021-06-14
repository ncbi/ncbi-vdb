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
 *  Memory manager creating non-swappable memory blocks
 */
class PinnedMemoryMgr : public TrackingBypassMemoryManager
{
public:
    /**
     * Interface for locking (pinning) memory block.
     */
    class MemoryLockerItf
    {
    public:
        virtual ~MemoryLockerItf () = 0;
        /**
         * Locks memory in the address range starting at ptr and continuing for bytes.
         * @param ptr pointer to the start of the address range
         * @param bytes size of the address range
         * @exception ???
         */
        virtual void lock( pointer ptr, size_type bytes ) = 0;
        /**
         * Unocks memory in the address range starting at ptr and continuing for bytes.
         * @param ptr pointer to the start of the address range
         * @param bytes size of the address range
         * @exception ???
         */
        virtual void unlock( pointer ptr, size_type bytes ) = 0;
    };

public:
    /**
     * Constructor
     * @param base_mgr optional pointer to a memory manager to handle allocation and deallocation. If nullptr, a default manager will be used.
     * @param locker optional pointer to a memory locker object. By default, uses an object that calls Posix mlock/munlock.
    */
    PinnedMemoryMgr( TrackingMemoryMgr base_mgr, MemoryLockerItf * locker = nullptr );

    /**
     * Constructor. Uses a default memory manager to handle allocation and deallocation.
     * @param locker optional pointer to a memory locker object. By default, uses an object that calls Posix mlock/munlock.
    */
    PinnedMemoryMgr( MemoryLockerItf * locker = nullptr );

    virtual ~PinnedMemoryMgr();

public: // inherited from MemoryManagerItf

    virtual void * reallocateUntracked ( void * block, bytes_t cur_size, bytes_t new_size );

    virtual pointer reallocate ( pointer ptr, size_type new_size );

protected:
    virtual void onAllocate ( void * ptr, size_type bytes );
    virtual void onDeallocate ( void * ptr, size_type bytes );

private:
    MemoryLockerItf &   m_locker; ///< memory locking object
};

} // namespace VDB3

