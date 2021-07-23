if( WIN32 )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:wmainCRTStartup")
    add_executable( test-link-md test-link )
    target_link_libraries( test-link-md ncbi-vdb-shared kapp )

    add_executable( test-link-mt test-link )
    MSVS_StaticRuntime( test-link-mt )
    target_link_libraries( test-link-mt  ncbi-vdb-shared kapp )

else()
    add_test( NAME Test_NCBI_VDB_vdb COMMAND ./so_check.sh ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libncbi-vdb.so WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

    add_test( NAME Test_NCBI_VDB_wvdb COMMAND ./so_check.sh ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libncbi-wvdb.so WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
endif()
