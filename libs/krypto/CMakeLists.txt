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

if( "unix" STREQUAL ${OS} )
    #include_directories(unix)
    set (OS_SPECIFIC_SRC unix/sysrng.c)
elseif ( "windows" STREQUAL ${OS} )
    #include_directories(win)
    set (OS_SPECIFIC_SRC win/sysrng.c)
endif()

#message("TEST KRYPTO")

#try_compile( TEST_RESULT "${CMAKE_CURRENT_SOURCE_DIR}/compile-test" compile-test.c )
#add_library( krypto STATIC compile-test.c )

set_source_files_properties( cipher-no-vec.vec no-null-ncbi.vec PROPERTIES COMPILE_FLAGS -DUSEVEC )
set_source_files_properties( cipher-no-vec.vecreg no-null-ncbi.vecreg PROPERTIES COMPILE_FLAGS -DUSEVECREG )
set_source_files_properties( cipher-no-vec.aes-ni no-null-ncbi.aes-ni PROPERTIES COMPILE_FLAGS -DUSEAESNI )

set( LIB_SRC
    ciphermgr
    wgaencrypt
    encfile
    encfilev2
    cipher
    aes-byte
    cipher-byte
    key
    reencfile
    rsa-aes-hmac
    no-null-ncbi
    cipher-no-vec.vec
    no-null-ncbi.vec
    cipher-no-vec.vecreg
    no-null-ncbi.vecreg
    cipher-no-vec.aes-ni
    no-null-ncbi.aes-ni
)

set( TEST_LIB_SRC
    testcipher
    
)

GenerateStaticLibs(krypto "${LIB_SRC}"
    #${OS_SPECIFIC_SRC}
)

GenerateStaticLibs( kryptotest "${TEST_LIB_SRC}" )

add_compile_definitions (__mod__="libs/krypto")