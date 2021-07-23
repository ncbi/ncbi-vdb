GenerateStaticLibs(ngs-py py_Manager )

add_compile_definitions (__mod__="libs/ngs-py")
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ngs )