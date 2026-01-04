cmake_minimum_required(VERSION 3.14)
project(etched VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

set(LIB_NAME etched)
add_library(${LIB_NAME} INTERFACE)
add_library(${LIB_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})

target_compile_features(${LIB_NAME} INTERFACE cxx_std_20)

target_include_directories(${LIB_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(ETCHED_BUILD_TESTS "Build etched tests" OFF)
option(ETCHED_BUILD_EXAMPLES "Build etched examples" ON)

if(ETCHED_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(ETCHED_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

include(CMakePackageConfigHelpers)

install(TARGETS ${LIB_NAME}
  EXPORT ${LIB_NAME}Targets
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT ${LIB_NAME}Targets
  FILE ${LIB_NAME}Targets.cmake
  NAMESPACE ${LIB_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME}
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${LIB_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME}
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME}
)

export(EXPORT ${LIB_NAME}Targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}Targets.cmake
  NAMESPACE ${LIB_NAME}::
)
