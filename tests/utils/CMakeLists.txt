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

add_executable(utils_tests
  ct_string_test.cpp
)
target_compile_options(utils_tests PRIVATE
  -Wall -Wextra -Wpedantic -Wshadow -Wconversion
  -Wold-style-cast -Wnon-virtual-dtor -Wunused
  -Woverloaded-virtual
)
target_link_libraries(utils_tests PRIVATE etched::etched GTest::gtest_main)
gtest_discover_tests(utils_tests)
