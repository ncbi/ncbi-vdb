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
    if( WIN32 )
        MSVS_StaticRuntime( ${target_name} )
        add_library( ${target_name}-md STATIC ${sources} )
        if(NOT "" STREQUAL "${compile_defs}" )
            target_compile_definitions( ${target_name}-md PRIVATE ${compile_defs} )
        endif()
        MSVS_DLLRuntime( ${target_name}-md )
    endif()
endfunction()

function( GenerateStaticLibs target_name sources )
   GenerateStaticLibsWithDefs( ${target_name} "${sources}" "" )
endfunction()


function( ExportStatic name install )
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
                    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib64
            )
         endif()
    else()
        set_target_properties( ${name} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG})
        set_target_properties( ${name} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE})
    endif()
endfunction()

#
# create versioned names and symlinks for a shared library
#
function(MakeLinksShared target name install)
    if( SINGLE_CONFIG )
        add_custom_command(TARGET ${target}
            POST_BUILD
            COMMAND rm -f lib${name}.${SHLX}.${VERSION}
            COMMAND mv lib${name}.${SHLX} lib${name}.${SHLX}.${VERSION}
            COMMAND ln -f -s lib${name}.${SHLX}.${VERSION} lib${name}.${SHLX}.${MAJVERS}
            COMMAND ln -f -s lib${name}.${SHLX}.${MAJVERS} lib${name}.${SHLX}
            WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        )

        set_property(
            TARGET    ${target}
            APPEND
            PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.${SHLX}.${VERSION};${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.${SHLX}.${MAJVERS};${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.${SHLX}"
        )

        if ( ${install} )
            install( FILES  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.${SHLX}.${VERSION}
                            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.${SHLX}.${MAJVERS}
                            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.${SHLX}
                    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib64
        )
        endif()
    endif()
endfunction()

#
# for a static library target, create a public shared target with the same base name and contents
#
function(ExportShared lib install)
    get_target_property( src ${lib} SOURCES )
    add_library( ${lib}-shared SHARED ${src} )
    MSVS_StaticRuntime( ${lib}-shared )
    set_target_properties( ${lib}-shared PROPERTIES OUTPUT_NAME ${lib} )
    MakeLinksShared( ${lib}-shared ${lib} ${install} )

    if( WIN32 )
        add_library( ${lib}-shared-md SHARED ${src} )
        MSVS_DLLRuntime( ${lib}-shared-md )
        set_target_properties( ${lib}-shared-md PROPERTIES OUTPUT_NAME ${lib}-md )
    endif()
endfunction()

function( BuildExecutableForTest exe_name sources libraries )
	add_executable( ${exe_name} ${sources} )
	MSVS_StaticRuntime( ${exe_name} )
	target_link_libraries( ${exe_name} ${libraries} )
endfunction()

function( AddExecutableTest test_name sources libraries )
	BuildExecutableForTest( "${test_name}" "${sources}" "${libraries}" )
	add_test( NAME ${test_name} COMMAND ${test_name} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
endfunction()