add_library( ncbi-ngs-c++ STATIC NCBI-NGS )

add_definitions (-D__mod__="libs/ngs-c++")
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ngs )

ExportStatic( ncbi-ngs-c++ )