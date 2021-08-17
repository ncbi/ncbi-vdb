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
 *  Memory manager tracking a specified allocation quota.
 *  Stops allocating if the quota is exceeded, with a chance for a subclass to increase the quota.
 *  Does not account for allocation overhead.
 */
class QuotaMemoryMgr : public TrackingBypassMemoryManager
{
public:
    /**
     * Constructor
     * @param base_mgr pointer to a memory manager to handle allocation and deallocation. If nullptr, a default manager will be used.
     * @param quota maximum number of bytes to be allocated by this instance at any given time
    */
    QuotaMemoryMgr( TrackingMemoryMgr base_mgr, bytes_t quota );

    /**
     * Constructor. Uses a default manager to handle allocation and deallocation.
     * @param quota maximum number of bytes to be allocated by this instance at any given time
    */
    QuotaMemoryMgr( bytes_t quota );

    virtual ~QuotaMemoryMgr();

public: // inherited from MemoryManagerItf

    /**
     * Allocate a memory block against the quota.
     * @param bytes see MemoryManagerItf
     * @return see MemoryManagerItf
     * @exception see MemoryManagerItf
     */
    virtual pointer allocate ( size_type bytes );

    virtual pointer reallocate ( pointer ptr, size_type new_size );

    virtual void * reallocateUntracked ( void * ptr, size_type old_size, size_type new_size );

protected:
    virtual void onAllocate ( void * ptr, size_type bytes );
    virtual void onDeallocate ( void * ptr, size_type bytes );

public:
    /**
     * Maximum total amount of memory to be allocated through this instance.
     * @return Maximum total amount of memory to be allocated through this instance, in bytes
     */
    bytes_t quota() const noexcept { return m_quota; }

    /**
     * A virtual method giving a subclass a chance to increase the quota once it has been exhausted.
     * @param min_extension minimum acceptable extension of the quota, in bytes
     * @return true if the quota extension has been granted; the new value of the quota is available through quota()
     */
    virtual bool update_quota( bytes_t min_extension ) { return false; }

    /**
     * Current amount of memory available for allocation.
     * @return Currently available memory, in bytes
     */
    bytes_t total_free() const noexcept { return m_quota - m_used; }

    /**
     * Current amount of memory allocated through this instance.
     * @return Currently allocated memory, in bytes
     */
    bytes_t total_used() const noexcept { return m_used; }

    /**
     * Maximum size of a block currently available for allocation.
     * @return Maximum size of a block currently available for allocation, in bytes. Not a guarantee of successful allocation.
     */
    bytes_t max_free() const noexcept { return total_free(); }

protected:
    /**
    * Update quota.
    * @param new_quota new value of the quota
    */
    void set_quota ( bytes_t new_quota ) { m_quota = new_quota; }

private:
    /**
    * Try to extend an exhausted quota.
     * @param min_extension minimum acceptable extension of the quota, in bytes
     * @exception std :: bad_alloc if cannot be extended TODO: switch to a VDB3 exception when implemented
    */
    void handle_quota_update( bytes_t min_extension );

    size_type   m_quota;    ///< current quota, in bytes
    size_type   m_used;     ///< total size of allocated blocks, in bytes
};

} // namespace VDB3

