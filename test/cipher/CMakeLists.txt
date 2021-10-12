if ( Python3_FOUND AND NOT WIN32 )
    if( "linux" STREQUAL ${OS} ) # fails on Teamcity/Mac due to apparent misconfiguration of Python
        add_test( NAME Test_CIPHER COMMAND ./test.sh ${CMAKE_SOURCE_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
        set_tests_properties(Test_CIPHER
            PROPERTIES ENVIRONMENT
                "PYTHON=${Python3_EXECUTABLE};LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
            )
    endif()
endif()
