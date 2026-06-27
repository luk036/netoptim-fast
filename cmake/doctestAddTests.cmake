# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying file Copyright.txt or
# https://cmake.org/licensing for details.

set(script "${CMAKE_CURRENT_LIST_DIR}/doctest.cmake")
set(prefix "${TEST_PREFIX}")
set(suffix "${TEST_SUFFIX}")
set(spec ${TEST_SPEC})
set(extra_args ${TEST_EXTRA_ARGS})
set(properties ${TEST_PROPERTIES})
set(add_labels ${TEST_ADD_LABELS})
set(junit_output_dir ${TEST_JUNIT_OUTPUT_DIR})
set(test_list ${TEST_LIST})

if("${CTEST_FILE}" STREQUAL "")
  if(CMAKE_CONFIGURATION_TYPES)
    set(CTEST_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_tests.cmake")
  else()
    set(CTEST_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_tests.cmake")
  endif()
endif()

function(add_command NAME)
  set(_args "")
  foreach(_arg ${ARGN})
    if(_arg MATCHES "[^-./:a-zA-Z0-9_]")
      set(_args "${_args} [==[${_arg}]==]")
    else()
      set(_args "${_args} ${_arg}")
    endif()
  endforeach()
  set(script
      "${script}add_test(${NAME}${_args})"
      PARENT_SCOPE
  )
endfunction()

function(set_command PROP)
  set(_args "")
  foreach(_arg ${ARGN})
    if(_arg MATCHES "[^-./:a-zA-Z0-9_]")
      set(_args "${_args} [==[${_arg}]==]")
    else()
      set(_args "${_args} ${_arg}")
    endif()
  endforeach()
  set(script
      "${script}set_tests_properties(${_args} PROPERTIES ${PROP} ${_args2})"
      PARENT_SCOPE
  )
endfunction()

# Run test executable to get list of available tests
if(NOT EXISTS "${TEST_EXECUTABLE}")
  message(FATAL_ERROR "Specified test executable '${TEST_EXECUTABLE}' does not exist")
endif()

execute_process(
  COMMAND ${TEST_EXECUTOR} "${TEST_EXECUTOR}" ${TEST_EXECUTABLE} --list-test-cases
  OUTPUT_VARIABLE output
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${TEST_WORKING_DIR}
)

if(NOT ${result} EQUAL 0)
  string(REPLACE "\n" "\n    " output "${output}")
  message(FATAL_ERROR "Error running test executable.\n" "  Path: '${TEST_EXECUTABLE}'\n"
                      "  Result: '${result}'\n" "  Output:\n" "    ${output}\n"
  )
endif()

# Parse output
foreach(line ${output})
  set(test ${line})
  # ... and add to script
  add_command(
    "${prefix}${test}${suffix}" ${TEST_EXECUTOR} "${TEST_EXECUTOR}" "${TEST_EXECUTABLE}" "${test}"
    ${extra_args}
  )
  set_command("${prefix}${test}${suffix}" PROPERTIES ${properties})
endforeach()

# Write test list file
file(WRITE "${CTEST_FILE}" "${script}")
