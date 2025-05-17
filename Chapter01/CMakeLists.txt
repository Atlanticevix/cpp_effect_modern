cmake_minimum_required(VERSION 3.10)

project(Chapter01)


set(SRC_DIR ${PROJECT_SOURCE_DIR})

file(GLOB_RECURSE SRC_FILES 
    ${SRC_DIR}/*.cpp 
    ${SRC_DIR}/*.c 
    ${SRC_DIR}/*.h 
    ${SRC_DIR}/*.hpp
)

add_executable(${PROJECT_NAME}  ${SRC_FILES})