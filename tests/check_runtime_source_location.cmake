execute_process(
  COMMAND "${LUCY_EXECUTABLE}" "${SOURCE_FILE}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE stdout
  ERROR_VARIABLE stderr
)
set(output "${stdout}${stderr}")
if(result EQUAL 0)
  message(FATAL_ERROR "Expected runtime NameError but execution succeeded: ${output}")
endif()
if(NOT output MATCHES "Lucy source:.*:2:[0-9]+")
  message(FATAL_ERROR "Expected exact runtime source line/column; got: ${output}")
endif()
if(NOT output MATCHES "2 \\| println missing_runtime_name")
  message(FATAL_ERROR "Expected source excerpt in diagnostic; got: ${output}")
endif()
if(NOT output MATCHES "\\^")
  message(FATAL_ERROR "Expected caret in diagnostic; got: ${output}")
endif()
message(STATUS "Runtime source location diagnostic passed")
