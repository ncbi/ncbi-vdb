#
# for a static library target, redirect its output to the public library
# location (.../lib) and create versioned names and symlinks
#
function(ExportStatic name )
    # the output goes to .../lib
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
endfunction()

#
# create versioned names and symlinks for a shared library
#
function(MakeLinksShared target name)
    add_custom_command(TARGET ${target}
        POST_BUILD
        COMMAND rm -f lib${name}.so.${VERSION}
        COMMAND mv lib${name}.so lib${name}.so.${VERSION}
        COMMAND ln -f -s lib${name}.so.${VERSION} lib${name}.so.${MAJVERS}
        COMMAND ln -f -s lib${name}.so.${MAJVERS} lib${name}.so
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    )
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
