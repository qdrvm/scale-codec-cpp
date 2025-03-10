#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

# Connects CMake options with vcpkg features
function (feature_option variable feature_name help_text default)
    if(PACKAGE_MANAGER STREQUAL "vcpkg" AND ${feature_name} IN_LIST VCPKG_MANIFEST_FEATURES)
        set(${variable} ON CACHE BOOL ${help_text} FORCE)
    else()
        set(${variable} ${default} CACHE BOOL ${help_text})
    endif()
endfunction()
