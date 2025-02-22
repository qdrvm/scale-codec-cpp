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

namespace scale::impl {

  /**
   * @namespace memory
   * @brief Memory-based implementation of SCALE encoding and decoding.
   */
  namespace memory {
    using EncoderToVector = backend::ToBytes<std::vector<uint8_t>>;

    /**
     * @brief Encodes a value using SCALE encoding.
     *
     * @tparam T The type of the value to encode.
     * @param value The value to encode.
     * @return A result containing the encoded byte vector or an error.
     */
    template <typename Out, typename T>
      requires backend::ByteReceiver<Out>
    outcome::result<Out> encode(T &&value) {
      Out out;
      backend::ToBytes<Out> encoder(out);
      try {
        // Allways send encoding value by const-lvalue-reference
        encode(static_cast<const std::remove_reference_t<T> &>(value), encoder);
      } catch (std::system_error &e) {
        return outcome::failure(e.code());
      }
      return std::move(out);
    }

    template <typename T>
    inline auto encode(T &&value) {
      return encode<std::vector<uint8_t>>(std::forward<T>(value));
    }

    using DecoderFromSpan = backend::FromBytes<std::span<const uint8_t>>;

    /**
     * @brief Decodes a value using SCALE decoding.
     *
     * @tparam T The type of the value to decode.
     * @param bytes The byte span containing encoded data.
     * @return A result containing the decoded value or an error.
     */
    template <typename T>
    outcome::result<T> decode(backend::ByteSource auto bytes) {
      backend::FromBytes<decltype(bytes)> decoder{bytes};
      T value{};
      try {
        decode(value, decoder);
      } catch (std::system_error &e) {
        return outcome::failure(e.code());
      }
      return std::move(value);
    }

    using EncoderForCount = backend::ForCount;

    /**
     * @brief Emulates encoding of value using SCALE, and count bytes
     *
     * @tparam T The type of the value to encode.
     * @param value The value to encode.
     * @return A result size of encoding result or an error.
     */
    template <typename T>
    outcome::result<size_t> encoded_size(T &&value) {
      EncoderForCount encoder;
      try {
        // Allways send encoding value by const-lvalue-reference
        encode(static_cast<const std::remove_reference_t<T> &>(value), encoder);
      } catch (std::system_error &e) {
        return outcome::failure(e.code());
      }
      return std::move(encoder).size();
    }

  }  // namespace memory

}  // namespace scale::impl
