/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file scale.hpp
 * @brief Combines all headers to use existing or build custom SCALE
 * serialization and deserialization mechanisms.
 */

#pragma once

#include <qtils/outcome.hpp>

#include <scale/decoder.hpp>
#include <scale/encoder.hpp>

#include <scale/backend/for_count.hpp>
#include <scale/backend/from_bytes.hpp>
#include <scale/backend/to_bytes.hpp>

#include <scale/detail/collections.hpp>
#include <scale/detail/compact_integer.hpp>
#include <scale/detail/decomposable.hpp>
#include <scale/detail/enum.hpp>
#include <scale/detail/fixed_width_integer.hpp>
#include <scale/detail/optional.hpp>
#include <scale/detail/smart_pointers.hpp>
#include <scale/detail/tagged.hpp>
#include <scale/detail/variant.hpp>

namespace scale {

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
  void decode(T &&value, ScaleDecoder auto &decoder)
    requires std::is_rvalue_reference_v<decltype(value)>
             and detail::HasLValueDecode<std::remove_reference_t<T>,
                                         decltype(decoder)>
  {
    decode(static_cast<std::remove_reference_t<T> &>(value), decoder);
  }
}  // namespace scale

namespace scale::impl {

  /**
   * @namespace memory
   * @brief Memory-based implementation of SCALE encoding and decoding.
   */
  namespace memory {
    using Encoder = Encoder<backend::ToBytes>;

    /**
     * @brief Encodes a value using SCALE encoding.
     *
     * @tparam T The type of the value to encode.
     * @param value The value to encode.
     * @return A result containing the encoded byte vector or an error.
     */
    template <typename T>
    outcome::result<std::vector<uint8_t>> encode(T &&value) {
      Encoder encoder;
      try {
        encode(std::forward<T>(value), encoder);
      } catch (std::system_error &e) {
        return outcome::failure(e.code());
      }
      return std::move(encoder).backend().to_vector();
    }

    using Decoder = Decoder<backend::FromBytes>;

    /**
     * @brief Decodes a value using SCALE decoding.
     *
     * @tparam T The type of the value to decode.
     * @param bytes The byte span containing encoded data.
     * @return A result containing the decoded value or an error.
     */
    template <typename T>
    outcome::result<T> decode(ConstSpanOfBytes bytes) {
      Decoder decoder{bytes};
      T value{};
      try {
        decode(value, decoder);
      } catch (std::system_error &e) {
        return outcome::failure(e.code());
      }
      return std::move(value);
    }
  }  // namespace memory

}  // namespace scale::impl
