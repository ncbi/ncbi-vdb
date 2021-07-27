if( WIN32 )
    add_executable( test-link-md test-link )
    MSVS_DLLRuntime( test-link-md )
    target_link_libraries( test-link-md ktst-md ncbi-vdb-shared-md kapp-md Ws2_32 )

    add_executable( test-link-mt test-link )
    MSVS_StaticRuntime( test-link-mt )
    target_link_libraries( test-link-mt ktst kapp ncbi-vdb-shared Ws2_32  )

    add_executable( test-wlink-md test-wlink )
    MSVS_DLLRuntime( test-wlink-md )
    target_link_libraries( test-wlink-md ktst-md ncbi-wvdb-shared-md kapp-md Ws2_32 )

    add_executable( test-wlink-mt test-wlink )
    MSVS_StaticRuntime( test-wlink-mt )
    target_link_libraries( test-wlink-mt ktst kapp ncbi-wvdb-shared Ws2_32  )
else()
    add_test( NAME Test_NCBI_VDB_vdb COMMAND ./so_check.sh ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libncbi-vdb.so WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

    add_test( NAME Test_NCBI_VDB_wvdb COMMAND ./so_check.sh ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libncbi-wvdb.so WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
endif()
