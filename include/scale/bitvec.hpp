/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines the BitVec struct for encoding and decoding bit vectors.
 *
 * This struct provides an encoding mechanism compatible with Rust's `BitVec<u8,
 * Lsb0>`. It supports efficient serialization and deserialization using SCALE
 * encoding.
 */

#pragma once

#include <vector>

#include <scale/detail/compact_integer.hpp>
#include <scale/detail/decomposable_type_traits.hpp>
#include <scale/types.hpp>

namespace scale {
  /**
   * @struct BitVec
   * @brief Represents a bit vector compatible with Rust's `BitVec<u8, Lsb0>`.
   *
   * This structure enables efficient storage and serialization of boolean
   * values using compact encoding.
   */
  struct BitVec : detail::decomposable::NoDecompose {
    /// @brief Stores the individual bits as a vector of boolean values.
    std::vector<bool> bits;

    /**
     * @brief Equality comparison operator.
     * @param other Another BitVec to compare against.
     * @return True if both BitVec instances contain the same bits, false
     * otherwise.
     */
    bool operator==(const BitVec &other) const {
      return bits == other.bits;
    }

    /**
     * @brief Encodes the bit vector into a SCALE encoder.
     * @param bit_vector The bit vector to encode.
     * @param encoder The encoder instance to write into.
     */
    friend void encode(const BitVec &bit_vector, Encoder &encoder) {
      encode(as_compact(bit_vector.bits.size()), encoder);
      size_t i = 0;
      uint8_t byte = 0;
      for (auto bit : bit_vector.bits) {
        if (bit) {
          byte |= 1 << (i % 8);
        }
        ++i;
        if (i % 8 == 0) {
          encoder.put(byte);
          byte = 0;
        }
      }
      if (i % 8 != 0) {
        encoder.put(byte);
      }
    }

    /**
     * @brief Decodes a BitVec from a SCALE decoder.
     * @param v The BitVec instance to populate.
     * @param decoder The decoder instance to read from.
     * @throws DecodeError::NOT_ENOUGH_DATA If there is insufficient data.
     */
    friend void decode(BitVec &v, Decoder &decoder) {
      size_t size;
      decode(as_compact(size), decoder);
      if (not decoder.has((size + 7) / 8)) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }
      v.bits.resize(size);
      size_t i = 0;
      uint8_t byte = 0;
      for (std::vector<bool>::reference bit : v.bits) {
        if (i % 8 == 0) {
          byte = decoder.take();
        }
        bit.operator=(((byte >> (i % 8)) & 1) != 0);
        ++i;
      }
    }
  };
}  // namespace scale
