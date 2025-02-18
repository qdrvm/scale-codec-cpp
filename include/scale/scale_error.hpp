/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief This file defines error codes for encoding and decoding operations
 * in the SCALE serialization format.
 *
 * This header provides error handling mechanisms for encoding and decoding
 * operations.
 */

#pragma once

#include <qtils/enum_error_code.hpp>

namespace scale {

  /**
   * @enum EncodeError
   * @brief Provides error codes for encoding methods in SCALE serialization.
   */
  enum class EncodeError {
    NEGATIVE_INTEGER,   ///< Encoding of negative integers is not supported.
    DEREF_NULLPOINTER,  ///< Attempt to dereference a null pointer.
    VALUE_TOO_BIG_FOR_COMPACT_REPRESENTATION  ///< Value is too large for
                                              ///< compact representation.
  };

  /**
   * @enum DecodeError
   * @brief Provides error codes for decoding methods in SCALE serialization.
   */
  enum class DecodeError {
    NOT_ENOUGH_DATA = 1,  ///< Not enough data to decode value.
    UNEXPECTED_VALUE,     ///< Unexpected value encountered during decoding.
    TOO_MANY_ITEMS,       ///< Too many items, cannot address them in memory.
    WRONG_TYPE_INDEX,     ///< Incorrect type index, cannot decode variant.
    INVALID_ENUM_VALUE,   ///< Enum value does not belong to the expected enum.
    REDUNDANT_COMPACT_ENCODING,  ///< Redundant bytes found in compact encoding.
    DECODED_VALUE_OVERFLOWS_TARGET  ///< Decoded value overflows the target
                                    ///< type.
  };

}  // namespace scale

OUTCOME_HPP_DECLARE_ERROR(scale, EncodeError)
OUTCOME_HPP_DECLARE_ERROR(scale, DecodeError)
