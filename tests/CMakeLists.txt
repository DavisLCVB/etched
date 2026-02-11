add_executable(component_tests
  component-tests.cpp
)
target_compile_options(component_tests PRIVATE
-Wall -Wextra -Wpedantic -Wshadow -Wconversion
-Wold-style-cast -Wnon-virtual-dtor -Wunused
-Woverloaded-virtual
)
target_link_libraries(component_tests PRIVATE etched::etched)
add_test(NAME component_tests COMMAND component_tests)
