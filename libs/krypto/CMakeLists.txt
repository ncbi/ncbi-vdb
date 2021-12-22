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
    aes_core
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