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

#include <memory/PrimordialMemoryMgr.hpp>

#include <memory/MemoryBlockItf.hpp>

using namespace VDB3;

/////////////// PrimordialMemoryMgr

PrimordialMemoryMgr::PrimordialMemoryMgr()
{
}

PrimordialMemoryMgr::~PrimordialMemoryMgr()
{
}

void *
PrimordialMemoryMgr::allocateUntracked ( bytes_t bytes )
{
    return allocate ( bytes );
}

void *
PrimordialMemoryMgr::reallocateUntracked ( void * block, bytes_t cur_size, bytes_t new_size )
{
    return reallocate ( block, new_size );
}

void
PrimordialMemoryMgr::deallocateUntracked ( void * block, bytes_t size ) noexcept
{
    return deallocate ( block, size );
}

MemoryManagerItf::pointer
PrimordialMemoryMgr::allocate ( size_type size )
{
    if ( size == 0 )
    {
        return nullptr;
    }
    pointer ret = ( pointer ) malloc ( size );
    if ( ret == nullptr )
    {
        throw std :: bad_alloc (); //TODO: use a VDB3 exception when implemented
    }
    return ret;
}

void
PrimordialMemoryMgr::deallocate ( pointer p, size_type bytes ) noexcept
{
    free ( p );
}

MemoryManagerItf::pointer
PrimordialMemoryMgr:: reallocate ( pointer p, size_type s )
{
    pointer ret = realloc ( p, s );
    if ( ret == nullptr && s != 0 )
    {
        throw std :: bad_alloc (); //TODO: use a VDB3 exception when implemented
    }
    return ret;
}

