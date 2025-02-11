/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Provides encoding and decoding utilities for tagged types in SCALE
 * serialization.
 *
 * This file defines functions for encoding and decoding tagged types,
 * ensuring proper serialization and deserialization while preserving type
 * information.
 */

#pragma once

#include <qtils/tagged.hpp>

#include <scale/detail/compact_integer.hpp>
#include <scale/types.hpp>

namespace scale {

  namespace detail::tagged {
    /// @brief Concept that ensures a type is not tagged.
    template <typename T>
    concept NoTagged = not qtils::is_tagged_v<T>;
  }  // namespace detail::tagged

  using detail::tagged::NoTagged;

  /**
   * @brief Encodes a tagged type using SCALE encoding.
   *
   * @tparam T The tagged type to encode.
   * @param tagged The value to encode.
   * @param encoder The encoder instance to write to.
   */
  template <typename T>
    requires qtils::is_tagged_v<T>
             and (not detail::compact_integer::CompactInteger<T>)
  void encode(const T &tagged, ScaleEncoder auto &encoder) {
    return encode(untagged(tagged), encoder);
  }

  /**
   * @brief Decodes a tagged type using SCALE decoding.
   *
   * @tparam T The tagged type to decode.
   * @param tagged The value to decode into.
   * @param decoder The decoder instance to read from.
   */
  template <typename T>
    requires qtils::is_tagged_v<T>
             and (not detail::compact_integer::CompactInteger<T>)
  void decode(T &tagged, ScaleDecoder auto &decoder) {
    return decode(untagged(tagged), decoder);
  }

}  // namespace scale
