#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

set(SCRIPT_PATH "${CMAKE_SOURCE_DIR}/scripts/generate_decompose_and_apply_hpp.sh")
set(DECOMPOSE_AND_APPLY_HPP_IN "${CMAKE_SOURCE_DIR}/include/scale/detail/decompose_and_apply.hpp.in")
set(DECOMPOSE_AND_APPLY_HPP "${CMAKE_BINARY_DIR}/include/scale/detail/decompose_and_apply.hpp")
add_custom_command(
    OUTPUT  ${DECOMPOSE_AND_APPLY_HPP}
    COMMAND ${CMAKE_COMMAND} -E echo "Running: ${SCRIPT_PATH} '${DECOMPOSE_AND_APPLY_HPP_IN}' '${MAX_AGGREGATE_FIELDS}' '${DECOMPOSE_AND_APPLY_HPP}'"
    COMMAND ${SCRIPT_PATH} ${DECOMPOSE_AND_APPLY_HPP_IN} ${MAX_AGGREGATE_FIELDS} ${DECOMPOSE_AND_APPLY_HPP}
    DEPENDS ${SCRIPT_PATH} ${DECOMPOSE_AND_APPLY_HPP_IN}
    COMMENT "Generating include/scale/detail/decompose_and_apply.hpp"
    VERBATIM
)
