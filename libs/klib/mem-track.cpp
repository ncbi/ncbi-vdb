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
#include <set>
#include <fstream>
#include <cassert>
#include <mutex>
#include <iostream>

using namespace std;

mutex guard;

typedef map< const void *, MemTrackBlockData > Blocks;
typedef map< string, set<const void*> > Names;

static Blocks blocks;
static Names names;

static uint64_t peak_memory = 0;
static uint64_t current_memory = 0;
const uint64_t MemoryThreshold = 100000000; //100M
//const uint64_t MemoryThreshold = 150000;

void Snapshot( std::ostream& out = cout )
{
    out << "peak_memory = " << peak_memory << endl;
//    out << "names.size = " << names.size() << endl;
    out << "source | blocks | total size | % " << endl;
    for (auto& b : names )
    {
        out << b.first << " | " << b.second.size() << " | ";
        size_t total = 0;
        for( auto& p : b.second )
        {
            total += blocks.find(p)->second.max_size;
        }
        out << total << " | " << ( total * 100 / peak_memory ) << endl;
        for( auto& p : b.second )
        {
            auto& b = blocks.find(p)->second;
            cout << "   " << b.name << " max_size=" << b.max_size << " created = " << b.created  << " freed = " << b.freed << endl;
        }
    }
    out << "==================================" << endl;
}

void UpdatePeak( uint64_t add, uint64_t subtract = 0 )
{
cout << "UpdatePeak(" << current_memory << ") +" << add << " -" << subtract << endl;
    current_memory += add;
    current_memory -= subtract;

    if ( current_memory > peak_memory )
    {
        peak_memory = current_memory;
        cout << "peak_memory= " << current_memory << endl;
        if ( peak_memory > MemoryThreshold )
        {
            Snapshot();
        }
    }
}

void MemTrackInit()
{
    guard.lock();

    blocks.clear();
    names.clear();
    guard.unlock();

    peak_memory = 0;
    current_memory = 0;
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
        UpdatePeak( size );
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
            UpdatePeak( size, b -> second . max_size );
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
cout << "MemTrackName=" << name << endl;
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
        names[ name ].insert( ptr );
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
        UpdatePeak( 0, b -> second . max_size );
        auto n = names.find( b -> second . name );
        if ( n != names.end() )
        {
            n -> second.erase( ptr );
        }
        blocks . erase( ptr );
    }

    guard.unlock();
}

const MemTrackBlockData * MemTrackGetBlock( const void * ptr )
{
    Blocks::const_iterator b = blocks.find( ptr );
    return ( b == blocks.end() ) ? nullptr : & ( b -> second );
}

void MemTrackDigest( std::ostream& out )
{
}
