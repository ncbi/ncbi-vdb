if ( Python3_FOUND )

    add_test(NAME Test_NGS_PY_import_ngs
        COMMAND ${Python3_EXECUTABLE} "-c" "from ngs import NGS"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_test(NAME SlowTest_NGS_PY
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    if( NOT NGS_LIBDIR )
        set( NGS_LIBDIR "" )
    endif()
    if (CMAKE_LIBRARY_OUTPUT_DIRECTORY)
        set_tests_properties(SlowTest_NGS_PY PROPERTIES
            ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}:${NGS_LIBDIR}")
    else() # a multi-config generator
        set_tests_properties(SlowTest_NGS_PY PROPERTIES
            ENVIRONMENT "LD_LIBRARY_PATH=$<CONFIG:Debug,${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}:$<CONFIG:Release,${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE}:>${NGS_LIBDIR}")
endif()
    ## seen in the original Makefile:
    # export VDB_CONFIG=$(VDB_CONFIG) ;
    # export NGS_PY_DOWNLOAD_LIBRARY=0;
    # unset NGS_PY_LIBRARY_PATH;
    # export LD_LIBRARY_PATH=$(LIBDIR):$(NGS_LIBDIR):$$LD_LIBRARY_PATH;
    # export NCBI_SETTINGS=/
    #$(PYTHON) tests.py

endif()
