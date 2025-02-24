/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines error categories for SCALE encoding and decoding.
 *
 * This file contains the implementation of error categories for
 * SCALE encoding and decoding errors, allowing them to be
 * used with the Outcome error handling library.
 */

#include <scale/scale_error.hpp>

/**
 * @brief Defines the error category for SCALE encoding errors.
 * @param e The specific encoding error.
 * @return A string describing the error.
 */
OUTCOME_CPP_DEFINE_CATEGORY(scale, EncodeError, e) {
  using scale::EncodeError;
  switch (e) {
    case EncodeError::NEGATIVE_INTEGER:
      return "SCALE encode: negative integers is not supported";
    case EncodeError::VALUE_TOO_BIG_FOR_COMPACT_REPRESENTATION:
      return "SCALE decode: value too big for compact representation";
    case EncodeError::DEREF_NULLPOINTER:
      return "SCALE encode: attempt to dereference a nullptr";
  }
  return "unknown EncodeError";
}

/**
 * @brief Defines the error category for SCALE decoding errors.
 * @param e The specific decoding error.
 * @return A string describing the error.
 */
OUTCOME_CPP_DEFINE_CATEGORY(scale, DecodeError, e) {
  using scale::DecodeError;
  switch (e) {
    case DecodeError::NOT_ENOUGH_DATA:
      return "SCALE decode: not enough data to decode";
    case DecodeError::UNEXPECTED_VALUE:
      return "SCALE decode: unexpected value occurred";
    case DecodeError::TOO_MANY_ITEMS:
      return "SCALE decode: collection has too many items or memory is out or "
             "data is damaged, unable to unpack";
    case DecodeError::WRONG_TYPE_INDEX:
      return "SCALE decode: wrong type index, cannot decode variant";
    case DecodeError::INVALID_ENUM_VALUE:
      return "SCALE decode: decoded enum value does not belong to the enum";
    case DecodeError::UNUSED_BITS_ARE_SET:
      return "SCALE decode: bits which must be unused have set";
    case DecodeError::REDUNDANT_COMPACT_ENCODING:
      return "SCALE decode: redundant bytes in compact encoding";
    case DecodeError::DECODED_VALUE_OVERFLOWS_TARGET:
      return "SCALE decode: encoded value overflows target type";
  }
  return "unknown SCALE DecodeError";
}
