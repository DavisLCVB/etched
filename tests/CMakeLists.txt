add_executable(etched_tests
  etched-tests.cpp
)
target_link_libraries(etched_tests PRIVATE etched::etched)

add_test(NAME etched_tests COMMAND etched_tests)
