#find_package (Python3 COMPONENTS Interpreter Development)

if ( Python3_FOUND )
add_test( NAME Test_NGS_PY COMMAND
${Python3_EXECUTABLE} "-c" "from ngs import NGS"
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests.py WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
# || echo 'NGS Python tests are not run since ngs module does not appear to be installed'
set_tests_properties(Test_NGS_PY PROPERTIES ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}:${NGS_LIBDIR})
#set_tests_properties(Test_NGS_PY PROPERTIES ENVIRONMENT NCBI_SETTINGS=/)

# && export VDB_CONFIG=$(VDB_CONFIG) ; export NGS_PY_DOWNLOAD_LIBRARY=0;
    # unset NGS_PY_LIBRARY_PATH;


endif()
