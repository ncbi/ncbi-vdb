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

#pragma once

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void MemTrackInit();
void MemTrackAlloc( const void * ptr, size_t size );
void MemTrackRealloc( const void * ptr, size_t size, const void * new_ptr );
void MemTrackName( const void * ptr, const char * name );
void MemTrackFree( const void * ptr );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <ios>
#include <string>

struct MemTrackBlockData
{
    std::string name;
    clock_t created;
    clock_t freed;
    size_t max_size;

    MemTrackBlockData( size_t orig_size);
};

const MemTrackBlockData * MemTrackGetBlock( const void * ptr );

void MemTrackDigest( std::ostream& out);

#endif
