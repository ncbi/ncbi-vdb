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
* =========================================================================== */

#include "mem-track.h"

#include <map>
#include <vector>
#include <fstream>
#include <cassert>
#include <mutex>

using namespace std;

mutex guard;

typedef map< const void *, MemTrackBlockData > Blocks;
typedef map< string, vector<const void*> > Names;
typedef multimap< const void *, MemTrackBlockData > FreedBlocks;

static Blocks blocks;
static FreedBlocks freed_blocks;
static Names names;

void MemTrackInit()
{
    guard.lock();

    blocks.clear();
    freed_blocks.clear();
    names.clear();
    guard.unlock();

}

MemTrackBlockData::MemTrackBlockData( size_t orig_size = 0 )
: created(clock()), freed(0), max_size( orig_size )
{
}

void MemTrackAlloc( const void * ptr, size_t size )
{
    guard.lock();

    Blocks::iterator b = blocks.find( ptr );
    if ( b == blocks.end() )
    {
        blocks[ ptr ] = MemTrackBlockData( size );
    }

    guard.unlock();
}

void MemTrackRealloc( const void * ptr, size_t size, const void * new_ptr )
{
    guard.lock();

    Blocks::iterator b = blocks.find( ptr );
    if ( b != blocks.end() )
    {
        if ( size != b -> second . max_size )
        {
            b -> second . max_size = size;
        }
        if ( new_ptr != ptr )
        {
            blocks[ new_ptr ] = b -> second;
            blocks.erase( ptr );
        }
    }

    guard.unlock();
}

void MemTrackName( const void * ptr, const char * name )
{
    guard.lock();
 
    Blocks::iterator b = blocks.find( ptr );
    if ( b == blocks.end() )
    {
        assert(false);
    }
    if ( b -> second . name . size() != 0 )
    {
        if ( b -> second . name != string(name) )
        {
            assert(false);
        }
        // same name, do nothing
    }
    else
    {
        b -> second . name = name;
        names[ name ].push_back( ptr );
    }

    guard.unlock();
}

void MemTrackFree( const void * ptr )
{
    guard.lock();

    Blocks::iterator b = blocks.find( ptr );
    if ( b != blocks.end() )
    {
        b -> second . freed = clock();
        freed_blocks . insert( *b );
        blocks . erase( ptr );
    }

    guard.unlock();
}

const MemTrackBlockData * MemTrackGetBlock( const void * ptr )
{
    Blocks::const_iterator b = blocks.find( ptr );
    return ( b == blocks.end() ) ? nullptr : & ( b -> second );
}

uint64_t
GetTotalSize( const void * ptr )
{   // sum up sizes of all allocations on this address
    auto range = freed_blocks.equal_range( ptr );
    uint64_t sum = 0;
    for( auto & b = range.first; b != range.second; ++b )
    {
        sum += b->second.max_size;
    }
    return sum;
}

void MemTrackDigest( std::ostream& out )
{
    guard.lock();

    out << "Memory Tracker Digest:" << endl;
    out << "source | blocks | total size" << endl;
    for (auto& b : names )
    {
        out << b.first << " | " << b.second.size() << " | ";
        size_t total = 0;
        for( auto& p : b.second )
        {
            total += GetTotalSize( p );
        }
        out << total << endl;
    }
    out << "==================================" << endl;
    out << "Unfreed blocks: :" << blocks.size() << endl;
    for (auto& b : blocks )
    {
        out << b.first << ": " << b.second.name << " " << b.second.max_size << endl;
    }

    guard.unlock();
}
