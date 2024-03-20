cmake_minimum_required(VERSION 3.28)
project(DraftMono)

add_subdirectory(src lib)

set(BUILD_TESTS ON CACHE BOOL "Build tests")
if(BUILD_TESTS)
    file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")
    file(GLOB_RECURSE TEST_HEADERS "test/*.hpp")

    add_executable(DraftMonoTest ${TEST_SOURCES} ${TEST_HEADERS})

    target_include_directories(DraftMonoTest PRIVATE src)
    target_link_libraries(DraftMonoTest Draft)
endif()

