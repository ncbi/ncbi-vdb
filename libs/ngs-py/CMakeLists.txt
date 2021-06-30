add_library(ngs-py STATIC py_Manager )

add_definitions (-D__mod__="libs/ngs-py")
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ngs )