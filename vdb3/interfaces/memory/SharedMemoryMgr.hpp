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

#include <map>

namespace VDB3
{

/**
 *  Memory manager creating memory blocks (possibly memory-mapped files) that can be shared between processes.
 */
class SharedMemoryMgr : public TrackingMemoryManager
{
public:
    /**
     * Constructor
    */
    SharedMemoryMgr();

    virtual ~SharedMemoryMgr();

public: // inherited from MemoryManagerItf
    using TrackingMemoryManager :: allocate;
    virtual pointer reallocate ( pointer ptr, size_type new_size );

////// TODO: separate memory-mapped file from anonymous shared memory blocks
//
// public:
//     /**
//      * Memory-map a (portion of a) file.
//      * @param bytes length of the mapped portion, in bytes
//      * @param fd file descriptor of an open file to be memory-mapped
//      * @param offset offset of the mapped portion, in bytes from the beginning of the file
//      * @param canWrite true if the block is to be updateable
//      * @param shared true if the block is to be shared between processes
//      * @return see MemoryManagerItf
//      * @exception TODO ??? if failed
//      */
//     pointer allocate ( size_type bytes, int fd, bool canWrite = false, off_t offset = 0, bool shared = false );

public:
    class SharedMemoryMgr_Internal;
};

} // namespace VDB3

