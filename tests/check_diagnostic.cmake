execute_process(
    COMMAND "${LUCY_EXECUTABLE}" "${SOURCE_FILE}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output)
if(result EQUAL 0)
    message(FATAL_ERROR "Expected Lucy source to fail, but it exited successfully")
endif()
set(combined "${output}${error_output}")
if(NOT combined MATCHES "${EXPECTED_PATTERN}")
    message(FATAL_ERROR "Expected diagnostic pattern '${EXPECTED_PATTERN}'. Actual output: ${combined}")
endif()
