include_directories(external/googletest/googletest/include)
include_directories(external)
include_directories(interfaces)
include_directories(inc)

add_subdirectory(external)
add_subdirectory(platform)
add_subdirectory(tests)
