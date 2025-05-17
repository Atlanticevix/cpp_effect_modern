cmake_minimum_required(VERSION 3.10)

project(EMCP)

# add_subdirectory(third_party)

# 指定为C++20 版本
set(CMAKE_CXX_STANDARD 20)

# 设置指定的C++编译器版本是必须的，如果不设置，或者为OFF，则指定版本不可用时，会使用上一版本。
set(CMAKE_CXX_STANDARD_REQUIRED on)

#cmake设置默认CMAKE_BUILD_TYPE
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

option(TEST_VALIDATION_LAYER "Validation Layer" ON)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # *.dll *.so
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # *.lib *.a
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # *.exe


set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(ARCHITECTURE "x86")
else()
    set(ARCHITECTURE "x64")
endif()

# message(STATUS ${CMAKE_BINARY_DIR})
# message(STATUS ${CMAKE_SYSTEM_NAME})
# message(STATUS ${ARCHITECTURE})

# add_subdirectory(frameworks)
# add_subdirectory(src)

add_subdirectory(Chapter01)