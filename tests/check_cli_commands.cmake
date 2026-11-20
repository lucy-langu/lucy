if(NOT DEFINED LUCY_EXECUTABLE OR NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "LUCY_EXECUTABLE and SOURCE_DIR are required")
endif()

set(PROJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/cli-command-fixture")
set(BIN_DIR "${PROJECT_DIR}/bin")
file(REMOVE_RECURSE "${PROJECT_DIR}")
file(MAKE_DIRECTORY "${PROJECT_DIR}/tools")
file(WRITE "${PROJECT_DIR}/lucy.toml"
"[project]\nname = \"cli-fixture\"\nversion = \"0.1.0\"\n\n[commands]\nhello = \"tools/hello.lucy\"\n")
file(WRITE "${PROJECT_DIR}/tools/hello.lucy"
"import fs\nprintln \"hello from metadata\"\nprintln ARGV.length()\nprintln ARGV[0]\nprintln fs.exists(\"lucy.toml\")\n")

execute_process(
  COMMAND "${LUCY_EXECUTABLE}" commands
  WORKING_DIRECTORY "${PROJECT_DIR}"
  RESULT_VARIABLE list_result OUTPUT_VARIABLE list_out ERROR_VARIABLE list_err)
if(NOT list_result EQUAL 0 OR NOT list_out MATCHES "hello")
  message(FATAL_ERROR "lucy commands failed: ${list_out}${list_err}")
endif()

execute_process(
  COMMAND "${LUCY_EXECUTABLE}" run hello "argument with spaces" two
  WORKING_DIRECTORY "${PROJECT_DIR}"
  RESULT_VARIABLE run_result OUTPUT_VARIABLE run_out ERROR_VARIABLE run_err)
if(NOT run_result EQUAL 0 OR NOT run_out MATCHES "hello from metadata" OR NOT run_out MATCHES "argument with spaces" OR NOT run_out MATCHES "true")
  message(FATAL_ERROR "lucy run failed: ${run_out}${run_err}")
endif()

execute_process(
  COMMAND "${LUCY_EXECUTABLE}" install-binstubs "${BIN_DIR}"
  WORKING_DIRECTORY "${PROJECT_DIR}"
  RESULT_VARIABLE install_result OUTPUT_VARIABLE install_out ERROR_VARIABLE install_err)
if(NOT install_result EQUAL 0)
  message(FATAL_ERROR "binstub installation failed: ${install_out}${install_err}")
endif()

if(WIN32)
  set(STUB "${BIN_DIR}/hello.cmd")
  if(NOT EXISTS "${STUB}")
    message(FATAL_ERROR "Windows binstub was not created")
  endif()
  execute_process(COMMAND cmd /c "${STUB}" one two
    RESULT_VARIABLE stub_result OUTPUT_VARIABLE stub_out ERROR_VARIABLE stub_err)
else()
  set(STUB "${BIN_DIR}/hello")
  if(NOT EXISTS "${STUB}")
    message(FATAL_ERROR "POSIX binstub was not created")
  endif()
  execute_process(COMMAND "${STUB}" "argument with spaces" two
    RESULT_VARIABLE stub_result OUTPUT_VARIABLE stub_out ERROR_VARIABLE stub_err)
endif()
if(NOT stub_result EQUAL 0 OR NOT stub_out MATCHES "hello from metadata" OR NOT stub_out MATCHES "argument with spaces" OR NOT stub_out MATCHES "true")
  message(FATAL_ERROR "binstub execution failed: ${stub_out}${stub_err}")
endif()

# Duplicate command keys and missing source files must fail deterministically.
file(WRITE "${PROJECT_DIR}/lucy.toml"
"[commands]\nhello = \"tools/hello.lucy\"\nhello = \"tools/hello.lucy\"\n")
execute_process(COMMAND "${LUCY_EXECUTABLE}" commands
  WORKING_DIRECTORY "${PROJECT_DIR}"
  RESULT_VARIABLE duplicate_result OUTPUT_VARIABLE duplicate_out ERROR_VARIABLE duplicate_err)
if(duplicate_result EQUAL 0 OR NOT "${duplicate_err}" MATCHES "duplicate command")
  message(FATAL_ERROR "duplicate command metadata was not rejected: ${duplicate_out}${duplicate_err}")
endif()

file(WRITE "${PROJECT_DIR}/lucy.toml"
"[commands]\nescape = \"../outside.lucy\"\n")
execute_process(COMMAND "${LUCY_EXECUTABLE}" run escape
  WORKING_DIRECTORY "${PROJECT_DIR}"
  RESULT_VARIABLE escape_result OUTPUT_VARIABLE escape_out ERROR_VARIABLE escape_err)
if(escape_result EQUAL 0 OR NOT "${escape_err}" MATCHES "outside the project")
  message(FATAL_ERROR "project command path traversal was not rejected: ${escape_out}${escape_err}")
endif()

file(WRITE "${PROJECT_DIR}/lucy.toml"
"[commands]\nmissing = \"tools/not_found.lucy\"\n")
execute_process(COMMAND "${LUCY_EXECUTABLE}" run missing
  WORKING_DIRECTORY "${PROJECT_DIR}"
  RESULT_VARIABLE missing_result OUTPUT_VARIABLE missing_out ERROR_VARIABLE missing_err)
if(missing_result EQUAL 0 OR NOT "${missing_err}" MATCHES "missing file")
  message(FATAL_ERROR "missing command entry was not rejected: ${missing_out}${missing_err}")
endif()
