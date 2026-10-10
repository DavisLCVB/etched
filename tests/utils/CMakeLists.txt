include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG 063de7e9578f82b369302001269680b4b1553359  # v1.18.0
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

set(ETCHED_UTILS_TEST_WARNINGS
  -Wall -Wextra -Wpedantic -Wshadow -Wconversion
  -Wold-style-cast -Wnon-virtual-dtor -Wunused
  -Woverloaded-virtual
)

add_executable(utils_tests
  ct_string_test.cpp
  expected_test.cpp
  optional_test.cpp
)
target_compile_options(utils_tests PRIVATE ${ETCHED_UTILS_TEST_WARNINGS})
target_link_libraries(utils_tests PRIVATE etched::etched GTest::gtest_main)
gtest_discover_tests(utils_tests)

# Expected aliases std::expected when the standard library provides it
# (C++23). Build its tests a second time as C++23 to cover that path.
if("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  add_executable(utils_tests_cxx23
    expected_test.cpp
  )
  target_compile_features(utils_tests_cxx23 PRIVATE cxx_std_23)
  target_compile_options(utils_tests_cxx23 PRIVATE
    ${ETCHED_UTILS_TEST_WARNINGS})
  target_link_libraries(utils_tests_cxx23 PRIVATE
    etched::etched GTest::gtest_main)
  gtest_discover_tests(utils_tests_cxx23 TEST_PREFIX "cxx23.")
endif()

# Compile-fail tests: each source must fail to build, and the build output
# must contain the expected diagnostic so that unrelated errors do not pass.
function(etched_add_compile_fail_test name source expected_message)
  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE etched::etched)
  set_target_properties(${name} PROPERTIES
    EXCLUDE_FROM_ALL TRUE
    EXCLUDE_FROM_DEFAULT_BUILD TRUE
  )
  add_test(
    NAME ${name}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${name}
            --config $<CONFIGURATION>
  )
  set_tests_properties(${name} PROPERTIES
    PASS_REGULAR_EXPRESSION "${expected_message}"
  )
endfunction()

etched_add_compile_fail_test(ct_string_unterminated_array
  compile_fail/ct_string_unterminated_array.cpp
  "array is not null-terminated")
etched_add_compile_fail_test(ct_string_empty_array
  compile_fail/ct_string_empty_array.cpp
  "array is not null-terminated")
etched_add_compile_fail_test(ct_string_substr_out_of_bounds
  compile_fail/ct_string_substr_out_of_bounds.cpp
  "CTString::substr out of bounds")
