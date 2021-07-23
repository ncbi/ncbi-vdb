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

function( GenerateStaticLibs
            target_name
            #[[ followed by any number of source file names, accessed as ARGN, a CMake list ]])
    list(JOIN ARGN " " sources )
    cmake_language(EVAL CODE
        "add_library( ${target_name} STATIC ${sources} )" )
    if( WIN32 )
        MSVS_StaticRuntime( ${target_name} )
        add_library( ${target_name}-md STATIC ${sources} ) #TODO: use EVAL
        MSVS_DLLRuntime( ${target_name}-md )
    endif()
endfunction()

function(ExportStatic name )
    # the output goes to .../lib
    if( CMAKE_LIBRARY_OUTPUT_DIRECTORY )
        set_target_properties( ${name} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
    else
        if( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG )
            set_target_properties( ${name} PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG} )
        endif()
        if( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE )
            set_target_properties( ${name} PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE} )
        endif()
    endif()

    if( NOT WIN32 )
        set_target_properties( ${name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
        # make the output name versioned, create all symlinks
        add_custom_command(TARGET ${name}
            POST_BUILD
            COMMAND rm -f lib${name}.a.${VERSION}
            COMMAND mv ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${name}.a lib${name}.a.${VERSION}
            COMMAND ln -f -s lib${name}.a.${VERSION} lib${name}.a.${MAJVERS}
            COMMAND ln -f -s lib${name}.a.${MAJVERS} lib${name}.a
            COMMAND ln -f -s lib${name}.a lib${name}-static.a
            WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        )
    endif()
endfunction()

#
# create versioned names and symlinks for a shared library
#
function(MakeLinksShared target name)
    if( WIN32 )
        # TODO: maybe copy binaries to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        # also produce -md version
    else()
        add_custom_command(TARGET ${target}
            POST_BUILD
            COMMAND rm -f lib${name}.${SHLX}.${VERSION}
            COMMAND mv lib${name}.${SHLX} lib${name}.${SHLX}.${VERSION}
            COMMAND ln -f -s lib${name}.${SHLX}.${VERSION} lib${name}.${SHLX}.${MAJVERS}
            COMMAND ln -f -s lib${name}.${SHLX}.${MAJVERS} lib${name}.${SHLX}
            WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        )
    endif()
endfunction()

#
# for a static library target, create a public shared target with the same base name and contents
#
function(ExportShared lib )
    get_target_property( src ${lib} SOURCES )
    add_library( ${lib}-shared SHARED ${src} )
    set_target_properties( ${lib}-shared PROPERTIES OUTPUT_NAME ${lib} )
    MakeLinksShared( ${lib}-shared ${lib} )
endfunction()
