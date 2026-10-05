cmake_minimum_required(VERSION 3.16)
project(lucy VERSION 2.0.0 LANGUAGES CXX)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LUCY_ENABLE_SQLITE "Enable built-in SQLite support" OFF)

add_library(lucy_runtime SHARED
  src/lucy.cpp
  src/lexer.cpp
  src/parser.cpp
  src/runtime.cpp
  src/value.cpp
  src/phase2.cpp
  src/stdlib_native.cpp
  src/repl.cpp
  src/extension.cpp
)

target_include_directories(lucy_runtime PUBLIC include)

if(LUCY_ENABLE_SQLITE)
  find_package(SQLite3 REQUIRED)
  target_link_libraries(lucy_runtime PUBLIC SQLite::SQLite3)
  target_compile_definitions(lucy_runtime PUBLIC LUCY_HAS_SQLITE=1)
  message(STATUS "SQLite support ENABLED")
else()
  message(STATUS "SQLite support DISABLED (LUCY_ENABLE_SQLITE=OFF)")
endif()

if(UNIX AND NOT APPLE)
  target_link_libraries(lucy_runtime PRIVATE dl)
endif()

if(WIN32)
  # stdlib_native.cpp uses Winsock directly, so ws2_32 must be linked
  # into the runtime DLL, not only into the lucy executable.
  target_link_libraries(lucy_runtime PRIVATE ws2_32)
endif()

add_executable(lucy src/main.cpp)
target_link_libraries(lucy PRIVATE lucy_runtime)

# Keep the installed executable self-contained with respect to the runtime.
# On Unix the runtime is installed in <prefix>/lib next to <prefix>/bin/lucy.
if(UNIX AND NOT APPLE)
  set_target_properties(lucy PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
elseif(APPLE)
  set_target_properties(lucy PROPERTIES INSTALL_RPATH "@loader_path/../lib")
endif()

if(MSVC)
  target_compile_options(lucy PRIVATE /W4)
else()
  target_compile_options(lucy PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(WIN32)
  install(TARGETS lucy lucy_runtime RUNTIME DESTINATION . LIBRARY DESTINATION . ARCHIVE DESTINATION lib)
  install(DIRECTORY stdlib/ DESTINATION stdlib)
  install(DIRECTORY docs/ DESTINATION docs)
  install(DIRECTORY editors/ DESTINATION editors)
else()
  install(TARGETS lucy lucy_runtime RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
  install(DIRECTORY stdlib/ DESTINATION share/lucy/stdlib)
  install(DIRECTORY docs/ DESTINATION share/doc/lucy)
  install(DIRECTORY editors/ DESTINATION share/lucy/editors)
endif()
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY extensions/ DESTINATION extensions OPTIONAL)
install(DIRECTORY assets/ DESTINATION share/lucy/assets)

enable_testing()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
  file(GLOB LUCY_TESTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.lucy")
  foreach(test_file ${LUCY_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    if(test_name MATCHES "sqlite" AND NOT LUCY_ENABLE_SQLITE)
      continue()
    endif()
    add_test(NAME ${test_name} COMMAND lucy ${test_file})
    set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")
    if(test_name MATCHES "^(04_type_errors|06_type_error_struct_field|08_invalid_hex_suffix|09_invalid_hex_separator|10_invalid_hex_trailing_separator|11_invalid_binary_digit|12_invalid_binary_separator|13_invalid_binary_trailing_separator|14_invalid_decimal_separator)$")
      set_tests_properties(${test_name} PROPERTIES WILL_FAIL TRUE)
    endif()
  endforeach()
endif()

add_executable(lucy_repl_completion_test tests/repl_completion_test.cpp)
target_link_libraries(lucy_repl_completion_test PRIVATE lucy_runtime)
add_test(NAME repl_completion COMMAND lucy_repl_completion_test)

