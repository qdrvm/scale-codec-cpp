#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.12)

option(JAM_COMPATIBLE "Build compatible with JAM-codec" OFF)
option(CUSTOM_CONFIG_SUPPORT "Support custom config of coder" OFF)
set(MAX_AGGREGATE_FIELDS 20 CACHE STRING "Max number of aggregates fields (1..1000); for generation")

option(BUILD_TESTS "Whether to include the test suite in build" OFF)

if (PACKAGE_MANAGER)
    if(PACKAGE_MANAGER NOT MATCHES "^(hunter|vcpkg)$")
        message(FATAL_ERROR "PACKAGE_MANAGER must be set to 'hunter', 'vcpkg' or isn't set")
    endif ()
else ()
    set(PACKAGE_MANAGER "hunter")
    if (CMAKE_TOOLCHAIN_FILE)
        get_filename_component(ACTUAL_NAME ${CMAKE_TOOLCHAIN_FILE} NAME)
        if(ACTUAL_NAME STREQUAL "vcpkg.cmake")
            message(STATUS "vcpkg will be used because vcpkg.cmake has found")
            set(PACKAGE_MANAGER "vcpkg")
        endif ()
    endif ()
endif ()
message(STATUS "Selected package manager: ${PACKAGE_MANAGER}")

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.27")
    cmake_policy(SET CMP0144 NEW)
endif ()

if (PACKAGE_MANAGER STREQUAL "hunter")
    include("cmake/Hunter/init.cmake")
endif ()

if(BUILD_TESTS)
    if (PACKAGE_MANAGER STREQUAL "vcpkg")
        list(APPEND VCPKG_MANIFEST_FEATURES scale-tests)
    endif()
endif()

project(Scale LANGUAGES CXX VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (PACKAGE_MANAGER STREQUAL "hunter")
    hunter_add_package(Boost)
    find_package(Boost)
else()
    find_package(Boost CONFIG REQUIRED COMPONENTS endian multiprecision)
endif ()

if (PACKAGE_MANAGER STREQUAL "hunter")
    hunter_add_package(qtils)
endif ()
find_package(qtils CONFIG REQUIRED)

SET(JAM_COMPATIBILITY_ENABLED "${JAM_COMPATIBLE}")
set(CUSTOM_CONFIG_ENABLED "${CUSTOM_CONFIG_SUPPORT}")
configure_file("${CMAKE_SOURCE_DIR}/include/scale/definitions.hpp.in" "${CMAKE_BINARY_DIR}/include/scale/definitions.hpp")

add_subdirectory(src)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test ${CMAKE_BINARY_DIR}/test_bin)
endif ()

###############################################################################
#   INSTALLATION
###############################################################################

include(GNUInstallDirs)

install(TARGETS scale EXPORT scaleConfig
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_PREFIX}
)

install(TARGETS scale_append EXPORT scaleConfig
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
