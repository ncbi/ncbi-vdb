set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test-bin )
add_executable(hello-vdb3 main.cpp)

target_link_libraries ( hello-vdb3 cmn3 kfc3 utf8proc ${LQUADMATH} )
