cmake_minimum_required(VERSION 3.16)
project(lucy VERSION 2.0.7 LANGUAGES CXX)
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
  src/environment.cpp
  src/type_system.cpp
  src/value.cpp
  src/phase2.cpp
  src/stdlib_native.cpp
  src/repl.cpp
  src/extension.cpp
)

target_include_directories(lucy_runtime PUBLIC include)

# Windows headers define min/max macros that break std::min/std::max,
# including initializer-list and explicitly templated overloads.
if(WIN32)
  target_compile_definitions(lucy_runtime PRIVATE NOMINMAX)
endif()

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

# Paw is a first-class executable backed by Lucy source in tools/paw/.
# It shares the runtime library but has its own stable CLI entry point.
add_executable(paw src/main.cpp)
target_compile_definitions(paw PRIVATE LUCY_PAW_BUILD=1 LUCY_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(paw PRIVATE lucy_runtime)

# Keep the installed executable self-contained with respect to the runtime.
# On Unix the runtime is installed in <prefix>/lib next to <prefix>/bin/lucy.
if(UNIX AND NOT APPLE)
  set_target_properties(lucy paw PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
elseif(APPLE)
  set_target_properties(lucy paw PROPERTIES INSTALL_RPATH "@loader_path/../lib")
endif()

if(WIN32)
  # Embed the Lucy application icon into the Windows executable.
  # The .ico is kept beside the resource script so both MSVC and MinGW
  # resource compilers can resolve it reliably.
  # MinGW windres may pass the compiler's absolute path (often under
  # "Program Files") to its preprocessor without preserving Windows quoting.
  # Resolve the preprocessor through PATH instead of inheriting that path.
  if(MINGW)
    set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} --preprocessor=cpp")
  endif()
  enable_language(RC)
  target_sources(lucy PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/assets/lucy.rc")
  target_include_directories(lucy PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/assets")
  install(TARGETS lucy paw lucy_runtime RUNTIME DESTINATION . LIBRARY DESTINATION . ARCHIVE DESTINATION lib)
  install(DIRECTORY stdlib/ DESTINATION stdlib)
  install(DIRECTORY tools/paw/ DESTINATION tools/paw)
  install(DIRECTORY docs/ DESTINATION docs)
  install(DIRECTORY editors/ DESTINATION editors)
else()
  install(TARGETS lucy paw lucy_runtime RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
  install(DIRECTORY stdlib/ DESTINATION share/lucy/stdlib)
  install(DIRECTORY tools/paw/ DESTINATION share/lucy/tools/paw)
  install(DIRECTORY docs/ DESTINATION share/doc/lucy)
  install(DIRECTORY editors/ DESTINATION share/lucy/editors)
endif()
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY extensions/ DESTINATION extensions OPTIONAL)
install(DIRECTORY assets/ DESTINATION share/lucy/assets)
if(UNIX AND NOT APPLE)
  # Register Lucy in desktop environments and install its hicolor icon.
  install(FILES assets/lucy.desktop DESTINATION share/applications)
  install(FILES assets/lucy.png DESTINATION share/icons/hicolor/256x256/apps RENAME lucy.png)
endif()

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
    if(test_name MATCHES "^(04_type_errors|06_type_error_struct_field|08_invalid_hex_suffix|09_invalid_hex_separator|10_invalid_hex_trailing_separator|11_invalid_binary_digit|12_invalid_binary_separator|13_invalid_binary_trailing_separator|14_invalid_decimal_separator|23_invalid_csv_quotes|24_invalid_json_surrogate|25_invalid_csv_delimiter|39_invalid_json_escape|50_invalid_array_sort|51_invalid_map_fetch|52_invalid_number_even|53_invalid_array_reduce_arity)$")
      set_tests_properties(${test_name} PROPERTIES WILL_FAIL TRUE)
    endif()
  endforeach()
endif()

add_test(NAME project_local_package_resolution
  COMMAND lucy "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/project_packages/main.lucy")
set_tests_properties(project_local_package_resolution PROPERTIES
  ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")

add_executable(lucy_repl_completion_test tests/repl_completion_test.cpp)
target_link_libraries(lucy_repl_completion_test PRIVATE lucy_runtime)
add_test(NAME repl_completion COMMAND lucy_repl_completion_test)
set_tests_properties(repl_completion PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")


add_test(NAME name_error_suggestion
    COMMAND ${CMAKE_COMMAND}
        -DLUCY_EXECUTABLE=$<TARGET_FILE:lucy>
        -DSOURCE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/name_error_suggestion.lucy.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_name_error_suggestion.cmake)
set_tests_properties(name_error_suggestion PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")
set_tests_properties(31_private_access_rejected PROPERTIES WILL_FAIL TRUE)
add_test(NAME runtime_source_location
    COMMAND ${CMAKE_COMMAND}
        -DLUCY_EXECUTABLE=$<TARGET_FILE:lucy>
        -DSOURCE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/32_runtime_source_location.lucy.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_runtime_source_location.cmake)
set_tests_properties(runtime_source_location PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")

# Hardening tests use a deliberately small depth: MSVC Debug frames are substantially larger,
# and the test must validate the controlled error rather than approach the native stack limit.
set_tests_properties(32_runtime_hardening PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib;LUCY_MAX_CALL_DEPTH=8")

add_test(NAME integer_overflow_literal_diagnostic
    COMMAND ${CMAKE_COMMAND}
        -DLUCY_EXECUTABLE=$<TARGET_FILE:lucy>
        -DSOURCE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/integer_overflow_literal.lucy.txt
        -DEXPECTED_PATTERN=integer\ literal\ out\ of\ range
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_diagnostic.cmake)
set_tests_properties(integer_overflow_literal_diagnostic PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")

add_test(NAME deep_parentheses_diagnostic
    COMMAND ${CMAKE_COMMAND}
        -DLUCY_EXECUTABLE=$<TARGET_FILE:lucy>
        -DSOURCE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/deep_parentheses.lucy.txt
        -DEXPECTED_PATTERN=expression\ nesting\ exceeds
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_diagnostic.cmake)
set_tests_properties(deep_parentheses_diagnostic PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")

add_test(NAME cli_project_commands
    COMMAND ${CMAKE_COMMAND}
        -DLUCY_EXECUTABLE=$<TARGET_FILE:lucy>
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_cli_commands.cmake)
set_tests_properties(cli_project_commands PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")
add_test(NAME paw_local_package_workflow
  COMMAND ${CMAKE_COMMAND}
    -DLUCY_EXECUTABLE=$<TARGET_FILE:lucy>
    -DPAW_EXECUTABLE=$<TARGET_FILE:paw>
    -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/package-workflow-test
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_package_workflow.cmake)
set_tests_properties(paw_local_package_workflow PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")
add_test(NAME paw_project_scaffold
  COMMAND ${CMAKE_COMMAND}
    -DLUCY_EXECUTABLE=$<TARGET_FILE:lucy>
    -DPAW_EXECUTABLE=$<TARGET_FILE:paw>
    -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/scaffold-test
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_project_scaffold.cmake)
set_tests_properties(paw_project_scaffold PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")
add_test(NAME paw_root_stdlib_verify COMMAND paw stdlib verify)
set_tests_properties(paw_root_stdlib_verify PROPERTIES
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")

add_test(NAME paw_root_test_runner COMMAND paw test $<TARGET_FILE:lucy>)
set_tests_properties(paw_root_test_runner PROPERTIES
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")


add_test(NAME paw_cli_help COMMAND paw help)
add_test(NAME paw_cli_version COMMAND paw --version)
add_test(NAME paw_cli_new_project
  COMMAND ${CMAKE_COMMAND}
    -DPAW_EXECUTABLE=$<TARGET_FILE:paw>
    -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/paw-new-test
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_paw_new.cmake)
set_tests_properties(paw_cli_new_project PROPERTIES ENVIRONMENT "LUCY_PATH=${CMAKE_CURRENT_SOURCE_DIR}/stdlib")

add_executable(lucy_runtime_core_test tests/runtime_core_test.cpp)
target_link_libraries(lucy_runtime_core_test PRIVATE lucy_runtime)
target_include_directories(lucy_runtime_core_test PRIVATE src)
add_test(NAME runtime_core_contracts COMMAND lucy_runtime_core_test)

foreach(target lucy_runtime lucy paw lucy_repl_completion_test lucy_runtime_core_test)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endforeach()

option(LUCY_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer (GCC/Clang)" OFF)
if(LUCY_ENABLE_SANITIZERS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(target lucy_runtime lucy paw lucy_repl_completion_test lucy_runtime_core_test)
      target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
      target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endforeach()
  else()
    message(FATAL_ERROR "LUCY_ENABLE_SANITIZERS requires GCC or Clang")
  endif()
endif()
