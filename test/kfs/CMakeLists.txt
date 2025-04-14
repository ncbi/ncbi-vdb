# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

add_compile_definitions( __mod__="test/kfs" )

set( LIBS kapp "${COMMON_LIBS_READ}")

if( NOT WIN32 )
    AddExecutableTest( Test_KFS_kdf "kdf" "${LIBS}" )
endif()

AddExecutableTest( Test_KFS_base "kfstest" "${LIBS}" )
AddExecutableTest( Test_KFS_ramfile "ramfiletest" "${LIBS}" )
AddExecutableTest( Test_KFS_resolve "resolvetest" "${LIBS}" )

if( NOT WIN32 )
    AddExecutableTest( Test_KFS_link "test-link" "${LIBS}" )
endif()

AddExecutableTest( Test_KFS_cachetee "cacheteetest" "${LIBS}" )

if( NOT WIN32 ) # TODO: these fail on Windows
    AddExecutableTest( Test_KFS_cachetee2 "cachetee2test" "${LIBS}" )
    AddExecutableTest( SlowTest_KFS_cachetee3 "cachetee3test" "${LIBS}" )
endif()

AddExecutableTest( Test_KFS_lru_cache "lru_cache_test" "${LIBS}" )
AddExecutableTest( Test_KFS_md5 "md5test" "${LIBS}" )
AddExecutableTest( Test_KFS_appendfile "appendfiletest" "${LIBS}" )

if( NOT WIN32 )
    AddExecutableTest( Test_KFS_GZip "test-gzip" "${LIBS}" )
endif()
