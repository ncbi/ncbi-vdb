if ( Python3_FOUND )
    add_test( NAME SlowTest_CIPHER COMMAND ./test.sh ${CMAKE_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    set_tests_properties(SlowTest_CIPHER PROPERTIES ENVIRONMENT PYTHON=${Python3_EXECUTABLE})
endif()
