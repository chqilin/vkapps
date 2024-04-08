cmake_minimum_required(VERSION 3.22)


project(vkapps VERSION 1.0)


# Visual Studio 2022 doesn't support C++20
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED FALSE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework Cocoa -framework IOKit")


cmake_host_system_information(RESULT _OS_NAME QUERY OS_NAME)
message("_OS_NNAME = " ${_OS_NAME})

set(_PROJECT_NAME "${PROJECT_NAME}")
set(_PROJECT_DIR "${PROJECT_SOURCE_DIR}")
set(_BINARY_DIR "${PROJECT_BINARY_DIR}")
set(_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

message("_PROJECT_NAME = ${_PROJECT_NAME}")
message("_PROJECT_DIR = ${_PROJECT_DIR}")
message("_SOURCE_DIR = ${_SOURCE_DIR}")
message("_BINARY_DIR = ${_BINARY_DIR}")


include("001-glfw.cmake")
include("002-vulkan.cmake")