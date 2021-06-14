add_executable ( test-memory
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

target_link_libraries ( test-memory memory ${GTEST_LIBRARIES} )

# gtest white box test
add_test ( NAME MemoryManagerTest COMMAND test-memory )
