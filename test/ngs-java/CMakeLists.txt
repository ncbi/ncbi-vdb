find_package(Java COMPONENTS Development)

if ( Java_Development_FOUND )

####
# TODO: calculate properly
set( NGS_LIB_PATH "/home/boshkins/ncbi-outdir/ngs-sdk/linux/gcc/x86_64/dbg/lib" )
#set( VDB_LIB_PATH "/home/boshkins/ncbi-outdir/ncbi-vdb/linux/gcc/x86_64/dbg/lib" )

set( VDB_LIB_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

    include(UseJava)
    enable_testing()
    project ( ngs_test NONE )

    set( NGS_JAVA_JAR /home/boshkins/ncbi-outdir/ngs-java/jar/ngs-java.jar)
    set( JUNIT_JAR ${CMAKE_CURRENT_SOURCE_DIR}/../junit-4.11.jar )
    set( CLASSPATH ${NGS_JAVA_JAR}:${JUNIT_JAR} )
    set( CMAKE_JAVA_COMPILE_FLAGS "-d" "${CMAKE_CURRENT_BINARY_DIR}" "-classpath" "${JUNIT_JAR}")

    add_jar(ngs_test
        SOURCES ngs_test_lib_load.java ngs_test_CSRA1.java
        INCLUDE_JARS ${JUNIT_JAR} ${NGS_JAVA_JAR}
    )

    get_target_property(_jarFile ngs_test JAR_FILE)

    add_test(NAME TestNGS_lib_load COMMAND
        ${Java_JAVA_EXECUTABLE}
        "-cp" "${_jarFile}:${JUNIT_JAR}:${NGS_JAVA_JAR}"
        "org.junit.runner.JUnitCore"
        "gov.nih.nlm.ncbi.ngs.ngs_test_lib_load"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    add_test(NAME TestNGS_CSRA1 COMMAND
        ${Java_JAVA_EXECUTABLE}
        "-cp" "${_jarFile}:${JUNIT_JAR}:${NGS_JAVA_JAR}"
        "-Dvdb.System.loadLibrary=1"
        "-Dvdb.log=FINEST"
        "-Djava.library.path=${NGS_LIB_PATH}:${VDB_LIB_PATH}"
        "org.junit.runner.JUnitCore"
        "ngs_test_CSRA1"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

endif()
