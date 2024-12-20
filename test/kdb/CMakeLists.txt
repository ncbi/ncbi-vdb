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

add_compile_definitions( __mod__="test/kdb" )

if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
	set( TMPDIR ./data)
else()
    set( ADDITIONAL_LIBS "" )
	set( TMPDIR /tmp )
endif()

set( LIBS_READ kapp "${COMMON_LIBS_READ}")
set( LIBS_WRITE kapp "${COMMON_LIBS_WRITE}" "${ADDITIONAL_LIBS}")

# TODO: make sure runs are not cached in the user repository when running tests

AddExecutableTest( Test_KDB_Setup "makedb" "${LIBS_WRITE}" )

set_tests_properties(Test_KDB_Setup PROPERTIES FIXTURES_SETUP Kdb)

# AddExecutableTest( Test_KDB_Btree "test-btree" "${LIBS_READ}" ) #TODO: revive test-btree
# AddExecutableTest( Test_KDB_Rowset "rowsettest" "${LIBS_WRITE};${ADDITIONAL_LIBS}" ) # not in use

AddExecutableTest( Test_KDB_KRColumn "test-rcolumn" "${LIBS_READ}" )
AddExecutableTest( Test_KDB_KWColumn "test-wcolumn" "${LIBS_WRITE}" )

AddExecutableTest( Test_KDB_KRTable "test-rtable" "${LIBS_READ}" )
AddExecutableTest( Test_KDB_KWTable "test-wtable" "${LIBS_WRITE}" )

AddExecutableTest( Test_KDB_KRDatabase "test-rdatabase" "${LIBS_READ}" )
AddExecutableTest( Test_KDB_KWDatabase "test-wdatabase" "${LIBS_WRITE}" )

AddExecutableTest( Test_KDB_KRManager "test-rmanager" "${LIBS_READ}" )
AddExecutableTest( Test_KDB_KWManager "test-wmanager" "${LIBS_WRITE}" )

AddExecutableTest( Test_KDB_KRMetadata "test-rmetadata" "${LIBS_READ}" )
AddExecutableTest( Test_KDB_KWMetadata "test-wmetadata" "${LIBS_WRITE}" )

AddExecutableTest( Test_KDB_KRMDataNode "test-rmetanode" "${LIBS_READ}" )
set_tests_properties(Test_KDB_KRMDataNode PROPERTIES FIXTURES_REQUIRED Kdb)

AddExecutableTest( Test_KDB_KWMDataNode "test-wmetanode" "${LIBS_WRITE}" )

AddExecutableTest( Test_KDB_KRIndex "test-rindex" "${LIBS_READ}" )
AddExecutableTest( Test_KDB_KWIndex "test-windex" "${LIBS_WRITE}" )

AddExecutableTest( Test_KDB_Kdb "kdbtest" "${LIBS_READ}" )
AddExecutableTest( Test_KDB_Wkdb "wkdbtest" "${LIBS_WRITE};${ADDITIONAL_LIBS}" )

AddExecutableTest( Test_KDB_CopyColumn "VDB-4705" "${LIBS_WRITE};${ADDITIONAL_LIBS}" )
set_property(TEST Test_KDB_CopyColumn PROPERTY ENVIRONMENT "TMPDIR=${TMPDIR}")

AddExecutableTest( Test_KDB_CopyMeta "VDB-4706" "${LIBS_WRITE};${ADDITIONAL_LIBS}" )
set_property(TEST Test_KDB_CopyMeta PROPERTY ENVIRONMENT "TMPDIR=${TMPDIR}")

AddExecutableTest( Test_KDB_Index "VDB-5323" "${LIBS_WRITE};${ADDITIONAL_LIBS}" )
set_property(TEST Test_KDB_Index PROPERTY ENVIRONMENT "TMPDIR=${TMPDIR}")

AddExecutableTest( Test_KDB_pagemap "test-pagemap" "${LIBS_WRITE}" )

AddExecutableTest( Test_KDB_rblob "test-rblob" "${LIBS_READ}" )
AddExecutableTest( Test_KDB_wblob "test-wblob" "${LIBS_WRITE}" )
