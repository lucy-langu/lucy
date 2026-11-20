if(NOT DEFINED PAW_EXECUTABLE OR NOT DEFINED WORK_DIR)
  message(FATAL_ERROR "PAW_EXECUTABLE and WORK_DIR are required")
endif()
file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}/caller")
execute_process(
  COMMAND "${PAW_EXECUTABLE}" new paw_fixture
  WORKING_DIRECTORY "${WORK_DIR}/caller"
  RESULT_VARIABLE create_result OUTPUT_VARIABLE create_out ERROR_VARIABLE create_err)
if(NOT create_result EQUAL 0)
  message(FATAL_ERROR "paw new failed: ${create_out}${create_err}")
endif()
set(PROJECT_DIR "${WORK_DIR}/caller/paw_fixture")
foreach(required IN ITEMS "lucy.toml" "README.md" "src/main.lucy" "tests/smoke.lucy")
  if(NOT EXISTS "${PROJECT_DIR}/${required}")
    message(FATAL_ERROR "paw new omitted required path: ${required}")
  endif()
endforeach()
execute_process(
  COMMAND "${PAW_EXECUTABLE}" run test
  WORKING_DIRECTORY "${PROJECT_DIR}"
  RESULT_VARIABLE test_result OUTPUT_VARIABLE test_out ERROR_VARIABLE test_err)
if(NOT test_result EQUAL 0 OR NOT test_out MATCHES "smoke test passed")
  message(FATAL_ERROR "paw run test failed: ${test_out}${test_err}")
endif()
