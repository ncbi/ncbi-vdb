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

#include <memory/SharedMemoryMgr.hpp>

#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtest/gtest.h>

#include "MemoryManagerItf_Test.hpp"
#include "TrackingMemoryManagerItf_Test.hpp"

using namespace VDB3;
using namespace std;

// SharedMemoryMgr

// interface conformance

INSTANTIATE_TYPED_TEST_SUITE_P(SharedMemoryMgr_ItfConformance, MemoryManagerItf_Test, SharedMemoryMgr);
INSTANTIATE_TYPED_TEST_SUITE_P(SharedMemoryMgr_TrackingItfConformance, TrackingMemoryManagerItf_Test, SharedMemoryMgr);

TEST ( SharedMemoryMgr, Instantiate )
{
    SharedMemoryMgr mgr;
}

TEST ( SharedMemoryMgr, Sharing )
{
    SharedMemoryMgr mgr;
    const SharedMemoryMgr::size_type Size = 100;
    SharedMemoryMgr :: pointer p = mgr.allocate ( Size );
    memset (p, 0, Size );
    pid_t pid = fork();
    ASSERT_TRUE ( pid >= 0 );
    if ( pid == 0 )
    {   // child
        memset (p, 0xda, Size );
        exit(1);
    }

    wait ( nullptr );
    const byte_t* bp = (const byte_t*)p;
    for (int i = 0; i < Size; ++i)
    {
        ASSERT_EQ( byte_t ( 0xda ), bp [ i ] );
    }
}

//TODO: read-only vs read-write anonymous blocks

////// TODO: separate memory-mapped file from anonymous shared memory blocks
//
// TEST ( SharedMemoryMgr, MmappingFile_Full_ReadWrite )
// {
//     const string Content = "1234";
//     {
//         ofstream f( test_info_ -> name() );
//         f << Content << endl;
//         f. close();
//     }

//     int fd = open ( test_info_ -> name(), O_RDWR );

//     SharedMemoryMgr mgr;
//     SharedMemoryMgr :: pointer p = mgr.allocate ( Content . size (), fd, true );
//     ASSERT_EQ ( Content, string ( (const char*)p, Content . size () ) );

//     mgr . deallocate ( p, Content . size () );

//     ASSERT_EQ ( 0, close ( fd ) );
//     remove ( test_info_ -> name() );
// }

// TEST ( SharedMemoryMgr, MmappingFile_Full_ReadOnly )
// {
//     const string Content = "1234";
//     {
//         ofstream f( test_info_ -> name() );
//         f << Content << endl;
//         f. close();
//     }

//     int fd = open ( test_info_ -> name(), O_RDONLY );

//     SharedMemoryMgr mgr;
//     SharedMemoryMgr :: pointer p = mgr.allocate ( Content . size (), fd ); // full file read only is the default
//     ASSERT_EQ ( Content, string ( (const char*)p, Content . size () ) );

//     mgr . deallocate ( p, Content . size () );

//     ASSERT_EQ ( 0, close ( fd ) );
//     remove ( test_info_ -> name() );
// }
//TODO: portion of a file
//TODO: map a file, non-shared

