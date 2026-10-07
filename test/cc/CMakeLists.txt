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

add_compile_definitions( __mod__="test/cc" )

set( LIBS "${COMMON_LIBS_READ}")

AddExecutableTest( Test_CC_Asm "asm-test" "${LIBS}" )

AddExecutableTest( Test_CC_Atomic-dflt "test-atomic" "${LIBS}" )
AddExecutableTest( Test_CC_Atomic32 "test-atomic32" "${LIBS}" )
AddExecutableTest( Test_CC_Atomic-32 "test-atomic-32" "${LIBS}" )
AddExecutableTest( Test_CC_Atomic-i32 "test-atomic-i32" "${LIBS}" )
AddExecutableTest( Test_CC_Atomic-64 "test-atomic-64" "${LIBS}" )
AddExecutableTest( Test_CC_Atomic-i64 "test-atomic-i64" "${LIBS}" )

AddExecutableTest( Test_CC_Int128 "test-int128" "${LIBS}" )
