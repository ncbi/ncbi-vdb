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

if( WIN32 )
    set( ADDITIONAL_LIBS Crypt32 )
	set( TMPDIR ./data)
else()
    set( ADDITIONAL_LIBS "" )
	set( TMPDIR /tmp )
endif()

# TODO: make sure runs are not cached in the user repository when running tests

add_compile_definitions( __mod__="test/kdb" )

# AddExecutableTest( Test_KDB_Btree "test-btree" "${COMMON_LIBS_READ}" ) #TODO: revive test-btree
# AddExecutableTest( Test_KDB_Rowset "rowsettest" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" ) # not in use

AddExecutableTest( Test_KDB_KRColumn "test-rcolumn" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KDB_KWColumn "test-wcolumn" "${COMMON_LIBS_WRITE}" )

AddExecutableTest( Test_KDB_KRTable "test-rtable" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KDB_KWTable "test-wtable" "${COMMON_LIBS_WRITE}" )

AddExecutableTest( Test_KDB_KRDatabase "test-rdatabase" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KDB_KWDatabase "test-wdatabase" "${COMMON_LIBS_WRITE}" )

AddExecutableTest( Test_KDB_KRManager "test-rmanager" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KDB_KWManager "test-wmanager" "${COMMON_LIBS_WRITE}" )

AddExecutableTest( Test_KDB_KRMetadata "test-rmetadata" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KDB_KWMetadata "test-wmetadata" "${COMMON_LIBS_WRITE}" )

AddExecutableTest( Test_KDB_Kdb "kdbtest" "${COMMON_LIBS_READ}" )
AddExecutableTest( Test_KDB_Wkdb "wkdbtest" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )

AddExecutableTest( Test_KDB_CopyColumn "VDB-4705" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
set_property(TEST Test_KDB_CopyColumn PROPERTY ENVIRONMENT "TMPDIR=${TMPDIR}")

AddExecutableTest( Test_KDB_CopyMeta "VDB-4706" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
set_property(TEST Test_KDB_CopyMeta PROPERTY ENVIRONMENT "TMPDIR=${TMPDIR}")

AddExecutableTest( Test_KDB_Index "VDB-5323" "${COMMON_LIBS_WRITE};${ADDITIONAL_LIBS}" )
set_property(TEST Test_KDB_Index PROPERTY ENVIRONMENT "TMPDIR=${TMPDIR}")
