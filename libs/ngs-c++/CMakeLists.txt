GenerateStaticLibs( ncbi-ngs-c++ NCBI-NGS )

add_compile_definitions (__mod__="libs/ngs-c++")
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ngs )

ExportStatic( ncbi-ngs-c++ true )