if( "unix" STREQUAL ${OS} )
    #include_directories(unix)
    set (OS_SPECIFIC_SRC unix/sysrng.c)
elseif ( "win" STREQUAL ${OS} )
    #include_directories(win)
    set (OS_SPECIFIC_SRC win/sysrng.c)
endif()

add_library(krypto STATIC
    aes-byte.c
    #aes-impl.c
    #aes-ncbi-ni.c
    aes-ncbi.c
    aes.c
    aesx86.c
    aes_core.c
    aes_x86core.c
    blockcipher.c
    cipher-byte.c
    cipher-no-vec.c
    cipher-vec.c
    cipher.c
    ciphermgr.c
    ciphertest.c
    compile-test.c
    csprng.c
    encfile.c
    encfilev2.c
    key.c
    manager.c
    no-null-ncbi.c
    null-ncbi.c
    reencfile.c
    rng.c
    rsa-aes-hmac.c
    testcipher.c
    wgaencrypt.c
    ${OS_SPECIFIC_SRC}
)

add_definitions (-D__mod__="libs/krypto")