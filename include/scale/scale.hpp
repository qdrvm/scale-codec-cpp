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
    using EncoderToBytes = backend::ToBytes;

    /**
     * @brief Encodes a value using SCALE encoding.
     *
     * @tparam T The type of the value to encode.
     * @param value The value to encode.
     * @return A result containing the encoded byte vector or an error.
     */
    template <typename T>
    outcome::result<std::vector<uint8_t>> encode(T &&value) {
      EncoderToBytes encoder;
      try {
        encode(std::forward<T>(value), encoder);
      } catch (std::system_error &e) {
        return outcome::failure(e.code());
      }
      return std::move(encoder).to_vector();
    }

    using DecoderFromBytes = backend::FromBytes;

    /**
     * @brief Decodes a value using SCALE decoding.
     *
     * @tparam T The type of the value to decode.
     * @param bytes The byte span containing encoded data.
     * @return A result containing the decoded value or an error.
     */
    template <typename T>
    outcome::result<T> decode(ConstSpanOfBytes bytes) {
      DecoderFromBytes decoder{bytes};
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
