#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.12)

# Select package manager
if(PACKAGE_MANAGER)
  if(PACKAGE_MANAGER NOT MATCHES "^(hunter|vcpkg)$")
    message(FATAL_ERROR "PACKAGE_MANAGER must be set to 'hunter', 'vcpkg' or isn't set")
  endif()
else()
  set(PACKAGE_MANAGER "hunter")
  if(CMAKE_TOOLCHAIN_FILE)
    get_filename_component(ACTUAL_NAME ${CMAKE_TOOLCHAIN_FILE} NAME)
    if(ACTUAL_NAME STREQUAL "vcpkg.cmake")
      message(STATUS "vcpkg will be used because vcpkg.cmake has found")
      set(PACKAGE_MANAGER "vcpkg")
    endif()
  endif()
endif()
message(STATUS "Selected package manager: ${PACKAGE_MANAGER}")

# Default values of options
set(JAM_COMPATIBLE_DEFAULT OFF)
set(CUSTOM_CONFIG_SUPPORT_DEFAULT OFF)
set(BUILD_TESTS_DEFAULT OFF)
set(ASAN_DEFAULT OFF)
set(TSAN_DEFAULT OFF)
set(UBSAN_DEFAULT OFF)

# Adjust value by vcpkg manifest if any
if(PACKAGE_MANAGER STREQUAL "vcpkg")
  if("jam-compatibility" IN_LIST VCPKG_MANIFEST_FEATURES)
    set(JAM_COMPATIBLE_DEFAULT ON)
  endif()
  if("configurable-coding" IN_LIST VCPKG_MANIFEST_FEATURES)
    set(CUSTOM_CONFIG_SUPPORT_DEFAULT ON)
  endif()
  if("scale-tests" IN_LIST VCPKG_MANIFEST_FEATURES)
    set(BUILD_TESTS_DEFAULT ON)
  endif()
  if("asan" IN_LIST VCPKG_MANIFEST_FEATURES)
    set(ASAN_DEFAULT ON)
  endif()
  if("tsan" IN_LIST VCPKG_MANIFEST_FEATURES)
    set(TSAN_DEFAULT ON)
  endif()
  if("ubsan" IN_LIST VCPKG_MANIFEST_FEATURES)
    set(UBSAN_DEFAULT ON)
  endif()
endif()

# Init options
option(JAM_COMPATIBLE "Build compatible with JAM-codec" ${JAM_COMPATIBLE_DEFAULT})
option(CUSTOM_CONFIG_SUPPORT "Support custom config of coder" ${CUSTOM_CONFIG_SUPPORT_DEFAULT})
option(BUILD_TESTS "Whether to include the test suite in build" ${BUILD_TESTS_DEFAULT})
option(ASAN "Build tests with address sanitizer" ${ASAN_DEFAUL})
option(TSAN "Build tests with thread sanitizer" ${TSAN_DEFAUL})
option(UBSAN "Build tests with undefined behavior sanitizer" ${UBSAN_DEFAUL})
if(ASAN OR TSAN OR UBSAN)
  if(NOT BUILD_TESTS)
    message(FATAL_ERROR "Sanitizer required but build of test is not set")
  endif()
endif()

# Adjust vcpkg features by custom defined option (for deploy possible dependencies)
if(PACKAGE_MANAGER STREQUAL "vcpkg")
  if(BUILD_TESTS AND NOT "scale-tests" IN_LIST VCPKG_MANIFEST_FEATURES)
    list(APPEND VCPKG_MANIFEST_FEATURES "scale-tests")
  endif()
endif()

set(MAX_AGGREGATE_FIELDS 20 CACHE STRING "Max number of aggregates fields (1..1000); for generation")

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.27")
  cmake_policy(SET CMP0144 NEW)
endif()

if(PACKAGE_MANAGER STREQUAL "hunter")
  include("cmake/Hunter/init.cmake")
else()
  set(HUNTER_ENABLED OFF)
endif()

project(Scale LANGUAGES CXX VERSION 2.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(PACKAGE_MANAGER STREQUAL "hunter")
  hunter_add_package(Boost)
  find_package(Boost)
else()
  find_package(Boost CONFIG REQUIRED COMPONENTS endian multiprecision)
endif()

if(PACKAGE_MANAGER STREQUAL "hunter")
  hunter_add_package(qtils)
endif()
find_package(qtils CONFIG REQUIRED)

set(JAM_COMPATIBILITY_ENABLED "${JAM_COMPATIBLE}")
set(CUSTOM_CONFIG_ENABLED "${CUSTOM_CONFIG_SUPPORT}")
configure_file("${CMAKE_SOURCE_DIR}/include/scale/definitions.hpp.in" "${CMAKE_BINARY_DIR}/include/scale/definitions.hpp")

if(ASAN)
  message(STATUS "Address sanitizer will be used")
  add_compile_options(-fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer)
endif()
if(TSAN)
  message(STATUS "Thread sanitizer will be used")
  add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
  add_link_options(-fsanitize=thread -fno-omit-frame-pointer)
endif()
if(UBSAN)
  message(STATUS "Undefined behavior sanitizer will be used")
  add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=undefined -fno-omit-frame-pointer)
endif()

add_subdirectory(src)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(test ${CMAKE_BINARY_DIR}/test_bin)
endif()

###############################################################################
#   INSTALLATION
###############################################################################

include(GNUInstallDirs)

install(
    TARGETS scale EXPORT scaleConfig
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_PREFIX}
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/scale
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/scale
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/scaleConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/scaleConfigVersion.cmake
    DESTINATION share/scale
)

install(
    EXPORT scaleConfig
    DESTINATION share/scale
    NAMESPACE scale::
)
