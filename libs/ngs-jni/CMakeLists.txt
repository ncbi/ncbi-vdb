GenerateStaticLibs(ngs-jni
    jni_Manager
    jni_String
    jni_ErrorMsg
)

add_compile_definitions (__mod__="libs/ngs-jni")
include_directories( BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ngs )