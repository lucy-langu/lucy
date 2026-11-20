if(NOT DEFINED LUCY_EXECUTABLE OR NOT DEFINED PAW_EXECUTABLE OR NOT DEFINED SOURCE_DIR OR NOT DEFINED WORK_DIR)
  message(FATAL_ERROR "LUCY_EXECUTABLE, PAW_EXECUTABLE, SOURCE_DIR, and WORK_DIR are required")
endif()
file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

execute_process(
  COMMAND "${PAW_EXECUTABLE}" new fixture_app
  WORKING_DIRECTORY "${WORK_DIR}"
  RESULT_VARIABLE create_result OUTPUT_VARIABLE create_out ERROR_VARIABLE create_err)
if(NOT create_result EQUAL 0)
  message(FATAL_ERROR "Lucy project scaffold failed: ${create_out}${create_err}")
endif()

set(PROJECT_DIR "${WORK_DIR}/fixture_app")
foreach(required IN ITEMS "lucy.toml" "README.md" "src/main.lucy" "tests/smoke.lucy")
  if(NOT EXISTS "${PROJECT_DIR}/${required}")
    message(FATAL_ERROR "scaffold omitted required path: ${required}")
  endif()
endforeach()

execute_process(COMMAND "${PAW_EXECUTABLE}" run test
  WORKING_DIRECTORY "${PROJECT_DIR}"
  RESULT_VARIABLE test_result OUTPUT_VARIABLE test_out ERROR_VARIABLE test_err)
if(NOT test_result EQUAL 0 OR NOT test_out MATCHES "smoke test passed")
  message(FATAL_ERROR "scaffold test command failed: ${test_out}${test_err}")
endif()

execute_process(COMMAND "${PAW_EXECUTABLE}" run start
  WORKING_DIRECTORY "${PROJECT_DIR}"
  RESULT_VARIABLE start_result OUTPUT_VARIABLE start_out ERROR_VARIABLE start_err)
if(NOT start_result EQUAL 0 OR NOT start_out MATCHES "Hello from fixture_app")
  message(FATAL_ERROR "scaffold start command failed: ${start_out}${start_err}")
endif()

execute_process(
  COMMAND "${PAW_EXECUTABLE}" new invalid.name
  WORKING_DIRECTORY "${WORK_DIR}"
  RESULT_VARIABLE invalid_result OUTPUT_VARIABLE invalid_out ERROR_VARIABLE invalid_err)
if(invalid_result EQUAL 0)
  message(FATAL_ERROR "scaffold accepted an invalid project name")
endif()
