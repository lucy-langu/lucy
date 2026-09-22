cmake_minimum_required(VERSION 3.16)
project(lucy VERSION 1.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(lucy
  src/main.cpp
  src/lucy.cpp
  src/lexer.cpp
  src/parser.cpp
  src/runtime.cpp
  src/value.cpp
  src/phase2.cpp
  src/stdlib_native.cpp
  src/repl.cpp

)
target_include_directories(lucy PRIVATE include)

find_package(SQLite3 QUIET)
if(SQLite3_FOUND)
  target_link_libraries(lucy PRIVATE SQLite::SQLite3)
  target_compile_definitions(lucy PRIVATE LUCY_HAS_SQLITE=1)
endif()
if(MSVC)
  target_compile_options(lucy PRIVATE /W4)
else()
  target_compile_options(lucy PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(WIN32)
  target_link_libraries(lucy PRIVATE ws2_32)
  install(TARGETS lucy RUNTIME DESTINATION .)
  install(DIRECTORY stdlib/ DESTINATION stdlib)
  install(DIRECTORY docs/ DESTINATION docs)
  install(DIRECTORY editors/ DESTINATION editors)
else()
  install(TARGETS lucy RUNTIME DESTINATION bin)
  install(DIRECTORY stdlib/ DESTINATION share/lucy/stdlib)
  install(DIRECTORY docs/ DESTINATION share/doc/lucy)
  install(DIRECTORY editors/ DESTINATION share/lucy/editors)
endif()
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY assets/ DESTINATION share/lucy/assets)

enable_testing()
foreach(test_name smoke features advanced classes maps exceptions strings stdlib features2 help filesystem_test phase1 phase2 repl_test extended_stdlib language_1_0_1 lucy_native)
  add_test(NAME ${test_name} COMMAND lucy ${CMAKE_CURRENT_SOURCE_DIR}/tests/${test_name}.lucy)
endforeach()
set_tests_properties(smoke features advanced classes maps exceptions strings stdlib features2 help filesystem_test phase1 phase2 repl_test extended_stdlib language_1_0_1 lucy_native PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")
add_test(NAME argv COMMAND lucy ${CMAKE_CURRENT_SOURCE_DIR}/tests/argv.lucy alpha 42 "hello world")
set_tests_properties(argv PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")
