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

# if adding mbedtls to our source tree:
# git clone --branch v4.2.0 --single-branch --depth=1 --recurse-submodules  https://github.com/Mbed-TLS/mbedtls.git
# 	message( "Building local copy ext/mbedtls..." )
# 	set(MBEDTLS_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/configs/config-thread.h")
# 	set(TF_PSA_CRYPTO_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/mbedtls/configs/crypto-config-thread.h")

# building mbedtls out of our source:
set( MBEDTLS_URL "https://github.com/Mbed-TLS/mbedtls.git")
#set( MBEDTLS_TAG "v4.2.0")
set( MBEDTLS_TAG "v3.2.1")

include (ExternalProject)
ExternalProject_Add( MBEDTLS
	PREFIX ${CMAKE_BINARY_DIR}/libs/ext/mbedtls # this is where the sources go
	GIT_REPOSITORY ${MBEDTLS_URL}
	GIT_TAG ${MBEDTLS_TAG}
	UPDATE_COMMAND "" # No update as we use specific tag
	CMAKE_ARGS
	-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
	-DCMAKE_C_FLAGS:STRING=-Wno-documentation;-Wno-error
	-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}
	-DCMAKE_INSTALL_INCLUDEDIR:PATH=include
	-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
	-DENABLE_TESTING=OFF
	-DENABLE_PROGRAMS=OFF
	-DCMAKE_INSTALL_LIBDIR:PATH=${MBEDTLS_LIBDIR}

	# -DCMAKE_BUILD_TYPE:STRING=Release
	-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
	# -DCMAKE_INSTALL_MESSAGE:STRING=LAZY
	#INSTALL_COMMAND ""
)

# collect all object files in one place
add_custom_target(combine_mbedtls ALL

	# extract object files
	COMMAND ar -x ${MBEDTLS_LIBDIR}/libmbedcrypto.a
	COMMAND ar -x ${MBEDTLS_LIBDIR}/libmbedx509.a
	COMMAND ar -x ${MBEDTLS_LIBDIR}/libmbedtls.a

	# avoid name conflicts with our object files
	COMMAND rm -f mbedtls_*.c.o # remove old object files first
	COMMAND ls -1 *.c.o | xargs -I{} mv {} mbedtls_{}

	WORKING_DIRECTORY ${MBEDTLS_LIBDIR}
	DEPENDS MBEDTLS
)

#TODO: Windows