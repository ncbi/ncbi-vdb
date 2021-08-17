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

#include <memory/RawMemoryBlock.hpp>
#include <memory/UniqueRawMemoryBlock.hpp>

#include "MemoryManagerItf_Test.hpp"
#include "TrackingMemoryManagerItf_Test.hpp"

using namespace std;
using namespace VDB3;

// FillingMemoryMgr

// inteface conformance
INSTANTIATE_TYPED_TEST_SUITE_P(FillingMemoryMgr_ItfConformance, MemoryManagerItf_Test, FillingMemoryMgr);
INSTANTIATE_TYPED_TEST_SUITE_P(FillingMemoryMgr_TrackingItfConformance, TrackingMemoryManagerItf_Test, FillingMemoryMgr);

const byte_t Filler = byte_t ( 0x5a );
const byte_t Trash = byte_t ( 0xde );

TEST ( FillingMemoryMgr, InstantiateDefaults_fillByte_trashByte )
{
    FillingMemoryMgr mgr;
    ASSERT_EQ ( FillingMemoryMgr::DefaultFiller, mgr.fillByte() );
    ASSERT_EQ ( FillingMemoryMgr::DefaultTrash,  mgr.trashByte() );
}

TEST ( FillingMemoryMgr, Instantiate )
{
    FillingMemoryMgr mgr ( Filler, Trash );
    ASSERT_EQ ( Filler, mgr.fillByte() );
    ASSERT_EQ ( Trash, mgr.trashByte() );
}

TEST ( FillingMemoryMgr, Allocate_WithFill )
{
    FillingMemoryMgr mgr ( Filler );

    auto ptr = mgr.allocate ( 2 );
    ASSERT_NE ( nullptr, ptr );
    ASSERT_EQ ( Filler, * ( byte_t * ) ptr );
    ASSERT_EQ ( Filler, * ( ( byte_t * ) ptr + 1 ) );

    mgr . deallocate ( ptr, 2 );
}

TEST ( FillingMemoryMgr, Rellocate_GrowWithFill )
{
    FillingMemoryMgr mgr ( Filler );

    auto ptr = mgr.allocate ( 2 );
    ptr = mgr.reallocate( ptr, 4 );
    ASSERT_EQ ( Filler, * ( byte_t * ) ptr );
    ASSERT_EQ ( Filler, * ( ( byte_t * ) ptr + 1 ) );
    ASSERT_EQ ( Filler, * ( ( byte_t * ) ptr + 2 ) );
    ASSERT_EQ ( Filler, * ( ( byte_t * ) ptr + 3 ) );

    mgr . deallocate ( ptr, 4 );
}

/**
 * A memory manager for testing filling of blocks.
 * Holds on to memory blocks on deallocation so that their contents can be examined
*/
class FreeOnceMgr : public MemoryManagerItf
{
public:
    /**
     * Constructor
     * @param p_size - total size of memory for all allocated blocks
     */
    FreeOnceMgr( bytes_t p_size ) : memory ( ( byte_t *) malloc ( p_size ) ), size ( p_size ), next_free ( 0 ) {}
    virtual ~FreeOnceMgr() { free ( memory ); }

    virtual void * allocateUntracked ( bytes_t bytes ) { return allocate(bytes); }
    virtual void * reallocateUntracked ( void* block, bytes_t cur_size, bytes_t new_size )  { assert(false); }
    virtual void deallocateUntracked ( void* block, bytes_t size ) noexcept {};

    virtual pointer allocate ( size_type bytes )
    {
        assert ( next_free + bytes < size );
        pointer ret = memory + next_free;
        next_free += bytes;
        return ret;
    }
    virtual pointer reallocate ( pointer ptr, size_type new_size ) { assert(false); }
    virtual void deallocate ( pointer ptr, size_type bytes ) noexcept {}

    byte_t * memory; ///< memory segment to allocate blocks from
    bytes_t size; ///< size of 'memory'
    size_type next_free; ///< next allocated block will start at this offset into 'memory'
};

TEST ( FillingMemoryMgr, Rellocate_ShrinkWithTrash )
{
    auto fom = make_shared < FreeOnceMgr > ( 1000 );
    auto tm = make_shared < TrackingMemoryManager > ( fom );
    FillingMemoryMgr mgr ( tm, Filler, Trash );

    auto ptr = mgr.allocate ( 4 );
    auto new_ptr = mgr.reallocate( ptr, 2 );
    ASSERT_EQ ( Filler, * ( ( byte_t * ) new_ptr ) );
    ASSERT_EQ ( Filler, * ( ( byte_t * ) new_ptr + 1 ) );
    ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr + 2 ) );
    ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr + 3 ) );

    mgr . deallocate ( new_ptr, 2 );
}

TEST ( FillingMemoryMgr, Rellocate_MoveWithTrash )
{
    auto fom = make_shared < FreeOnceMgr > ( 1000 );
    auto tm = make_shared < TrackingMemoryManager > ( fom );
    FillingMemoryMgr mgr ( tm, Filler, Trash );

    auto ptr = mgr.allocate ( 2 );
    auto new_ptr = mgr.reallocate( ptr, 4 ); // this will always move the block
    ASSERT_NE ( ptr, new_ptr ); // make sure it has been moved
    ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr ) );
    ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr + 1 ) );

    mgr . deallocate ( new_ptr, 4 );
}

TEST ( FillingMemoryMgr, Deallocate_WithTrash )
{
    auto fom = make_shared < FreeOnceMgr > ( 1000 );
    auto tm = make_shared < TrackingMemoryManager > ( fom );
    FillingMemoryMgr mgr ( tm, Filler, Trash );

    auto ptr = mgr.allocate ( 2 );
    mgr . deallocate ( ptr, 2 );
    ASSERT_EQ ( Trash, * ( byte_t * ) ptr );
    ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr + 1 ) );
}

// functionality is not lost when using the VDB-facing (no tracking) API

TEST ( FillingMemoryMgr, Alloc_NoTracking )
{
    auto mgr = make_shared < FillingMemoryMgr > ( Filler, Trash );

    {   // block sizes are not tracked by the manager but filling on allocation still occurs
        RawMemoryBlock rmb ( mgr, 2 );  // uses mgr -> allocateNoTracking()
        ASSERT_THROW( mgr -> getBlockSize( rmb . ptr () ), logic_error  ); //TODO: use VDB3 exception type
        ASSERT_NE ( nullptr, rmb . data() );
        ASSERT_EQ ( Filler, * rmb . data() );
        ASSERT_EQ ( Filler, * ( rmb . data() + 1 ) );
    }
}

TEST ( FillingMemoryMgr, Dealloc_NoTracking )
{
    auto fom = make_shared < FreeOnceMgr > ( 1000 );
    auto tm = make_shared < TrackingMemoryManager > ( fom );
    auto mgr = make_shared < FillingMemoryMgr > ( tm, Filler, Trash );

    const byte_t * ptr;
    {   // block sizes are not tracked by the manager but trashing on deallocation still occurs
        RawMemoryBlock rmb ( mgr, 2 );
        ptr = rmb . data();
    }   // the destructor uses mgr -> deallocateNoTracking()

    ASSERT_EQ ( Trash, * ( byte_t * ) ptr );
    ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr + 1 ) );
}

TEST ( FillingMemoryMgr, Realloc_NoTracking )
{
    auto fom = make_shared < FreeOnceMgr > ( 1000 );
    auto tm = make_shared < TrackingMemoryManager > ( fom );
    auto mgr = make_shared < FillingMemoryMgr > ( tm, Filler, Trash );

    const byte_t * ptr;
    {   // block sizes are not tracked by the manager but filling/trashing on reallocation still occurs
        UniqueRawMemoryBlock rmb ( mgr, 2 ); // UniqueRawMemoryBlock is resizeable
        ptr = rmb . data();
        ASSERT_NE ( nullptr, rmb . data() );
        ASSERT_EQ ( Filler, * rmb . data() );
        ASSERT_EQ ( Filler, * ( rmb . data() + 1 ) );
        rmb . data () [0] = byte_t ( 0 );
        rmb . data () [1] = byte_t ( 1 );

        rmb . resize ( 3 );
        // the block has been moved, filled
        ASSERT_NE ( nullptr, rmb . data() );
        ASSERT_NE ( ptr, rmb . data() );
        ASSERT_EQ ( byte_t ( 0 ), * rmb . data() );
        ASSERT_EQ ( byte_t ( 1 ), * ( rmb . data() + 1 ) );
        ASSERT_EQ ( Filler, * ( rmb . data() + 2 ) );

        // old block trashed
        ASSERT_EQ ( Trash, * ( byte_t * ) ptr );
        ASSERT_EQ ( Trash, * ( ( byte_t * ) ptr + 1 ) );
    }
}