cmake_minimum_required(VERSION 3.16)
project(lucy_extensions_examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(lucy CONFIG QUIET)

# Build from the Lucy source tree: configure this directory with
# -DLUCY_SOURCE_DIR=/path/to/lucy and -DLUCY_RUNTIME_DIR=/path/to/build.
set(LUCY_SOURCE_DIR "" CACHE PATH "Lucy source tree")
set(LUCY_RUNTIME_DIR "" CACHE PATH "Lucy build directory")

if(NOT LUCY_SOURCE_DIR OR NOT LUCY_RUNTIME_DIR)
  message(FATAL_ERROR "Set LUCY_SOURCE_DIR and LUCY_RUNTIME_DIR")
endif()

add_library(lucy_hello MODULE hello.cpp)
target_include_directories(lucy_hello PRIVATE "${LUCY_SOURCE_DIR}/include")
target_link_directories(lucy_hello PRIVATE "${LUCY_RUNTIME_DIR}")
target_link_libraries(lucy_hello PRIVATE lucy_runtime)
set_target_properties(lucy_hello PROPERTIES PREFIX "lib" OUTPUT_NAME "lucy_hello")
