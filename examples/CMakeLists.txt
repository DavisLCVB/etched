# CMakeLists.txt for etched examples

file(GLOB EXAMPLE_SOURCES "*.cpp")

foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
  get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
  add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
  target_link_libraries(${EXAMPLE_NAME} PRIVATE etched::etched)
  target_compile_features(${EXAMPLE_NAME} PRIVATE cxx_std_20)
  target_compile_options(${EXAMPLE_NAME} PRIVATE
  -Wall -Wextra -Wpedantic -Wshadow -Wconversion
  -Wold-style-cast -Wnon-virtual-dtor -Wunused
  -Woverloaded-virtual
  )
  set_target_properties(${EXAMPLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
  )
endforeach()
