cmake_minimum_required(VERSION 3.27)
project(Draft VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.hpp")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})
add_library(Draft STATIC ${SOURCES} ${HEADERS})
# Enable support for MSVC_RUNTIME_LIBRARY
cmake_policy(SET CMP0091 NEW)
# Use MTd for Debug and MT for Release
set_property(TARGET Draft PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(ZYAN_ZYCORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../Deps/zyanCore")
add_subdirectory(../Deps/zasm zasm)
target_link_libraries(Draft PRIVATE zasm)
target_include_directories(Draft PUBLIC ../Deps/zasm/include)