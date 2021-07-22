if(CMAKE_TEST_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_TEST_OUTPUT_DIRECTORY} )
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test-bin )
endif()

add_executable ( test-memory3
    test-FillingMemoryMgr.cpp
    test-MemoryBlockItf.cpp
    test-MemoryManagerItf.cpp
    test-PinnedMemoryMgr.cpp
    test-PrimordialMemoryMgr.cpp
    test-QuotaMemoryMgr.cpp
    test-RawMemoryBlock.cpp
    test-SharedMemoryMgr.cpp
    test-TrackingMemoryManager.cpp
    test-TypedMemoryBlock.cpp
    test-UniqueRawMemoryBlock.cpp
    test-main.cpp
)

target_link_libraries ( test-memory3 memory3 gtest )

# gtest white box test
add_test ( NAME Test_Memory3 COMMAND test-memory3 )
