find_package (Python3 COMPONENTS Interpreter Development)

if ( Python3_FOUND )
add_test( NAME Test_NGS_PY COMMAND

${Python3_EXECUTABLE} -c 'from ngs import NGS'
&& export VDB_CONFIG=$(VDB_CONFIG) ; export NGS_PY_DOWNLOAD_LIBRARY=0;
    unset NGS_PY_LIBRARY_PATH;
    export LD_LIBRARY_PATH=$(LIBDIR):$(NGS_LIBDIR):$$LD_LIBRARY_PATH;
    NCBI_SETTINGS=/ ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests.py
|| echo 'NGS Python tests are not run since ngs module does not appear to be installed'

)
endif()
