execute_process(
    COMMAND "${LUCY_EXECUTABLE}" "${SOURCE_FILE}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output)
if(result EQUAL 0)
    message(FATAL_ERROR "Expected undefined variable to fail, but Lucy exited successfully")
endif()
set(combined "${output}${error_output}")
if(NOT combined MATCHES "NameError: undefined variable 'usernme'. Did you mean 'username'\\?")
    message(FATAL_ERROR "Expected typo suggestion in diagnostic. Actual output: ${combined}")
endif()
