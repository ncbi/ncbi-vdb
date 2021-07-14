if ( Python3_FOUND )

    add_test(NAME Test_NGS_PY_import_ngs
        COMMAND ${Python3_EXECUTABLE} "-c" "from ngs import NGS"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_test(NAME SlowTest_NGS_PY
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(SlowTest_NGS_PY PROPERTIES
        ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}:${NGS_LIBDIR}")

    ## seen in the original Makefile:
    # export VDB_CONFIG=$(VDB_CONFIG) ;
    # export NGS_PY_DOWNLOAD_LIBRARY=0;
    # unset NGS_PY_LIBRARY_PATH;
    # export LD_LIBRARY_PATH=$(LIBDIR):$(NGS_LIBDIR):$$LD_LIBRARY_PATH;
    # export NCBI_SETTINGS=/
    #$(PYTHON) tests.py

endif()
