/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief This file defines the fundamental components of SCALE encoding and
 * decoding.
 *
 * SCALE (Simple Concatenated Aggregate Little-Endian) is a lightweight,
 * efficient, and deterministic serialization format used in blockchain
 * applications. This header provides the primary interfaces and type
 * definitions for encoding and decoding operations.
 */

#pragma once

#include <ranges>
#include <span>
#include <type_traits>
#include <vector>

#include <qtils/tagged.hpp>

#include <scale/decoder.hpp>
#include <scale/encoder.hpp>

namespace scale {

  /// @brief Convenience alias for arrays of bytes.
  using ByteArray = std::vector<uint8_t>;

  /// @brief Convenience alias for immutable span of bytes.
  using ConstSpanOfBytes = std::span<const uint8_t>;

  /// @brief Convenience alias for mutable span of bytes.
  using MutSpanOfBytes = std::span<uint8_t>;

  namespace detail {
    /**
     * @brief Concept to check if a type supports lvalue decoding.
     *
     * @tparam T The type being checked.
     * @tparam D The decoder type.
     */
    template <typename T, typename D>
    concept HasLValueDecode = requires(T value, D &decoder) {
      { decode(value, decoder) };
    };
  }  // namespace detail

  /**
   * @brief Decodes a value using SCALE decoding.
   *
   * @tparam T The type of the value to decode.
   * @tparam decoder The decoder used for decoding.
   * @param value The rvalue reference to the value being decoded.
   */
  template <typename T>
  void decode(T &&value, Decoder &decoder)
    requires std::is_rvalue_reference_v<decltype(value)>
             and detail::HasLValueDecode<std::remove_reference_t<T>,
                                         decltype(decoder)>
  {
    decode(static_cast<std::remove_reference_t<T> &>(value), decoder);
  }
}  // namespace scale
