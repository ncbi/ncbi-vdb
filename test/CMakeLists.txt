if(CMAKE_TEST_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_TEST_OUTPUT_DIRECTORY} )
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test-bin )
endif()

add_subdirectory(kfc)

link_libraries( ktst kapp )

# for the tests that link against libncbi-[w]vdb
set( COMMON_LIBS_READ   ncbi-vdb  dl pthread )
set( COMMON_LIBS_WRITE  ncbi-wvdb dl pthread )

add_subdirectory(ktst)
add_subdirectory(cc)
add_subdirectory(ext)
add_subdirectory(judy_test)
add_subdirectory(klib)
add_subdirectory(kfs)
add_subdirectory(kfg)
add_subdirectory(kapp)
add_subdirectory(cloud)
add_subdirectory(kns) # TODO: EXT_TOOLS - see the kns CMakeLists.txt for the details
add_subdirectory(krypto)
add_subdirectory(cipher)
add_subdirectory(kdb)
add_subdirectory(kproc)
add_subdirectory(vdb)
add_subdirectory(search)
add_subdirectory(sraxf)
add_subdirectory(vxf)
add_subdirectory(loader)
add_subdirectory(schema) # TODO: INT_TOOLS - see the schema CMakeLists.txt for the details
add_subdirectory(vfs)
add_subdirectory(ncbi-vdb)
add_subdirectory(ngs-python)
add_subdirectory(ngs-java)
# add_subdirectory(install)