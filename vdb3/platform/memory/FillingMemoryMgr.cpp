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

#include <memory/FillingMemoryMgr.hpp>

// memset
#include <cstring>
#include <cassert>

#include <memory/PrimordialMemoryMgr.hpp>

using namespace VDB3;

const byte_t VDB3 :: FillingMemoryMgr :: DefaultFiller;
const byte_t VDB3 :: FillingMemoryMgr :: DefaultTrash;

FillingMemoryMgr :: FillingMemoryMgr( TrackingMemoryMgr base_mgr, byte_t fill_byte, byte_t trash_byte )
:   TrackingBypassMemoryManager ( base_mgr ),
    m_fillByte ( fill_byte ),
    m_trashByte ( trash_byte )
{
}

FillingMemoryMgr :: FillingMemoryMgr( byte_t fill_byte, byte_t trash_byte )
:   m_fillByte ( fill_byte ),
    m_trashByte ( trash_byte )
{
}

FillingMemoryMgr :: ~FillingMemoryMgr()
{
}

void
FillingMemoryMgr :: onAllocate ( void * ptr, size_type bytes )
{
    if ( ptr != nullptr )
    {
        memset( ptr, int ( m_fillByte ), bytes );
    }
}

void
FillingMemoryMgr :: onDeallocate ( void * ptr, size_type bytes )
{
    if ( ptr != nullptr )
    {
        memset( ptr, int ( m_trashByte ), bytes );
    }
}

void *
FillingMemoryMgr :: reallocateUntracked( void * old_ptr, bytes_t old_size, bytes_t new_size )
{
    if ( old_ptr == nullptr )
    {
        return allocateUntracked ( new_size );
    }
    if ( new_size == 0 )
    {
        deallocateUntracked ( old_ptr, old_size );
        return nullptr;
    }

    // cannot not rely on the underlying manager's reallocate() since we need control over
    // position and size of the block in order to fill/trash properly

    // Always move the block since we need to trash the original block
    // and by the time the underlying manager is done reallocating, the original block may be inaccessible.
    void * new_ptr = allocateUntracked ( new_size );
    memmove ( new_ptr, old_ptr, std :: min ( old_size, new_size ) );
    deallocateUntracked ( old_ptr, old_size );

    return new_ptr;
}

FillingMemoryMgr :: pointer
FillingMemoryMgr :: reallocate ( pointer old_ptr, size_type new_size )
{
    if ( old_ptr == nullptr )
    {
        return allocate ( new_size );
    }

    size_type old_size = getBlockSize ( old_ptr );

    if ( new_size == 0 )
    {
        deallocate ( old_ptr, old_size );
        return nullptr;
    }

    // cannot not rely on the underlying manager's reallocate() since we need control over
    // position and size of the block in order to fill/trash properly

    // Always move the block since we need to trash the original block
    // and by the time the underlying manager is done reallocating, the original block may be inaccessible.
    pointer new_ptr = allocate ( new_size );
    memmove ( new_ptr, old_ptr, std :: min ( old_size, new_size ) );
    deallocate ( old_ptr, old_size );

    return new_ptr;
}

