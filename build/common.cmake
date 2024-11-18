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

#
# for a static library target, redirect its output to the public library
# location (.../lib) and create versioned names and symlinks
#

function( MSVS_StaticRuntime name )
    if( WIN32 )
        set_property(TARGET ${name} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endfunction()

function( MSVS_DLLRuntime name )
    if( WIN32 )
        set_property(TARGET ${name} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
endfunction()

function( GenerateStaticLibsWithDefs target_name sources compile_defs )
    add_library( ${target_name} STATIC ${sources} )
    if( NOT "" STREQUAL "${compile_defs}" )
        target_compile_definitions( ${target_name} PRIVATE ${compile_defs} )
    endif()
    if( WIN32 AND NOT _NCBIVDB_CFG_PACKAGING)
        MSVS_StaticRuntime( ${target_name} )
        add_library( ${target_name}-md STATIC ${sources} )
        if(NOT "" STREQUAL "${compile_defs}" )
            target_compile_definitions( ${target_name}-md PRIVATE ${compile_defs} )
        endif()
        MSVS_DLLRuntime( ${target_name}-md )
    endif()

    if( RUN_SANITIZER_TESTS )
        set( asan_defs "-fsanitize=address" "-fsanitize=undefined" )
        add_library( "${target_name}-asan" STATIC ${sources} )
        if( NOT "" STREQUAL "${compile_defs}" )
            target_compile_definitions( "${target_name}-asan" PRIVATE ${compile_defs} )
        endif()
        target_compile_options( "${target_name}-asan" PUBLIC ${asan_defs} )
        target_link_options( "${target_name}-asan" PUBLIC ${asan_defs} )

        set( tsan_defs "-fsanitize=thread" "-fsanitize=undefined" )
        add_library( "${target_name}-tsan" STATIC ${sources} )
        if( NOT "" STREQUAL "${compile_defs}" )
            target_compile_definitions( "${target_name}-tsan" PRIVATE ${compile_defs} )
        endif()
        target_compile_options( "${target_name}-tsan" PUBLIC ${tsan_defs} )
        target_link_options( "${target_name}-tsan" PUBLIC ${tsan_defs} )
    endif()
endfunction()

function( GenerateStaticLibs target_name sources )
   GenerateStaticLibsWithDefs( ${target_name} "${sources}" "" )
endfunction()


function( ExportStatic name install )
    if(_NCBIVDB_CFG_PACKAGING)
        return()
    endif()
    # the output goes to .../lib
    if( SINGLE_CONFIG )
        # make the output name versioned, create all symlinks
        set_target_properties( ${name} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
        add_custom_command(TARGET ${name}
            POST_BUILD
            COMMAND rm -f lib${name}.a.${VERSION}
            COMMAND mv lib${name}.a lib${name}.a.${VERSION}
            COMMAND ln -f -s lib${name}.a.${VERSION} lib${name}.a.${MAJVERS}
            COMMAND ln -f -s lib${name}.a.${MAJVERS} lib${name}.a
            COMMAND ln -f -s lib${name}.a lib${name}-static.a
            WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        )

        set_property(
            TARGET    ${name}
            APPEND
            PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.a.${VERSION};${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.a.${MAJVERS};${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.a;${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}-static.a"
        )

        if ( ${install} )
            install( FILES  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.a.${VERSION}
                            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.a.${MAJVERS}
                            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.a
                            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}-static.a
                    DESTINATION ${INST_LIBDIR}
            )
         endif()
    else()
        set_target_properties( ${name} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG})
        set_target_properties( ${name} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE})
        # if ( ${install} )
        #     install( TARGETS ${name} DESTINATION ${INST_LIBDIR} )
        # endif()
    endif()
endfunction()

#
# create versioned names and symlinks for a shared library
#
function(MakeLinksShared target name install)
    if(_NCBIVDB_CFG_PACKAGING)
        return()
    endif()
    set_target_properties( ${target} PROPERTIES OUTPUT_NAME ${name} )
    if( SINGLE_CONFIG )
        if( ${OS} STREQUAL "mac" )
            set( LIBSUFFIX ".${VERSION}.${SHLX}" )
            set( MAJLIBSUFFIX ".${MAJVERS}.${SHLX}" )
        else()
            set( LIBSUFFIX ".${SHLX}.${VERSION}" )
            set( MAJLIBSUFFIX ".${SHLX}.${MAJVERS}" )
        endif()
        add_custom_command(TARGET ${target}
            POST_BUILD
            COMMAND rm -f lib${name}${LIBSUFFIX}
            COMMAND mv lib${name}.${SHLX} lib${name}${LIBSUFFIX}
            COMMAND ln -f -s lib${name}${LIBSUFFIX} lib${name}${MAJLIBSUFFIX}
            COMMAND ln -f -s lib${name}${MAJLIBSUFFIX} lib${name}.${SHLX}
            WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        )

        set_property(
            TARGET    ${target}
            APPEND
            PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}${LIBSUFFIX};${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}${MAJLIBSUFFIX};${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.${SHLX}"
        )

        if ( ${install} )
            install( PROGRAMS  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}${LIBSUFFIX}
                            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}${MAJLIBSUFFIX}
                            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.${SHLX}
                    DESTINATION ${INST_LIBDIR}
        )
        endif()
    else()
        set_target_properties( ${target} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
        set_target_properties( ${target} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
        # if ( ${install} )
        #     install( TARGETS ${target}
        #              ARCHIVE DESTINATION ${INST_BINDIR}
        #              RUNTIME DESTINATION ${INST_BINDIR}
        #     )
        # endif()
    endif()
endfunction()

#
# for a static library target, create a public shared target with the same base name and contents
#
function(ExportShared lib install extra_libs)
    if(_NCBIVDB_CFG_PACKAGING)
        return()
    endif()
    get_target_property( src ${lib} SOURCES )
    add_library( ${lib}-shared SHARED ${src} )
    target_link_libraries( ${lib}-shared ${extra_libs} )
    MakeLinksShared( ${lib}-shared ${lib} ${install} )

    if( WIN32 )
        MSVS_StaticRuntime( ${lib}-shared )

        # on Windows, add a DLL built with /MD
        add_library( ${lib}-shared-md SHARED ${src} )
        target_link_libraries( ${lib}-shared-md ${extra_libs})
        MSVS_DLLRuntime( ${lib}-shared-md )
        MakeLinksShared( ${lib}-shared-md ${lib}-md ${install} )
    endif()
endfunction()

function( BuildExecutableForTest exe_name sources libraries )
	add_executable( ${exe_name} ${sources} )
        if(NOT _NCBIVDB_CFG_PACKAGING)
	    MSVS_StaticRuntime( ${exe_name} )
        endif()
	target_link_libraries( ${exe_name} ${libraries} )
endfunction()

function( AddExecutableTest test_name sources libraries )
	BuildExecutableForTest( "${test_name}" "${sources}" "${libraries}" )
	add_test( NAME ${test_name} COMMAND ${test_name} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

	if( RUN_SANITIZER_TESTS )
		set( asan_defs "-fsanitize=address" "-fsanitize=undefined" )
		BuildExecutableForTest( "${test_name}_asan" "${sources}" "${libraries}" )
		target_compile_options( "${test_name}_asan" PRIVATE ${asan_defs} )
		target_link_options( "${test_name}_asan" PRIVATE ${asan_defs} )
		add_test( NAME "${test_name}_asan" COMMAND "${test_name}_asan" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

		set( tsan_defs "-fsanitize=thread" "-fsanitize=undefined" )
		BuildExecutableForTest( "${test_name}_tsan" "${sources}" "${libraries}" )
		target_compile_options( "${test_name}_tsan" PRIVATE ${tsan_defs} )
		target_link_options( "${test_name}_tsan" PRIVATE ${tsan_defs} )
		add_test( NAME "${test_name}_tsan" COMMAND "${test_name}_tsan" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
	endif()
endfunction()

include(CheckIncludeFileCXX)
#check_include_file_cxx(mbedtls/md.h HAVE_MBEDTLS_H)
set(HAVE_MBEDTLS_H 0) # TODO: disabling system mbedtls since it may be outdated
set( MBEDTLS_LIBS mbedx509 mbedtls mbedcrypto ) # need to link against mbedtls in any case: system or local mbedtls
set(CMAKE_REQUIRED_LIBRARIES ${MBEDTLS_LIBS})
if ( HAVE_MBEDTLS_H )
	include(CheckCXXSourceRuns)
	check_cxx_source_runs("
#include <stdio.h>
#include \"mbedtls/md.h\"
#include \"mbedtls/sha256.h\"
int main(int argc, char *argv[]) {
	mbedtls_md_context_t ctx;
	mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
	mbedtls_md_init(&ctx);
	printf(\"test p: %p\", ctx.md_ctx);
}
" HAVE_MBEDTLS_F)
else()
    set(MBEDTLS_LIBS "")
endif()
