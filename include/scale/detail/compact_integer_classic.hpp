/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Provides encoding and decoding functions for compact integer
 *        representation using SCALE encoding.
 *
 * The CLASSIC compact integer encoding is a variable-length encoding scheme
 * optimized for small numbers while still supporting large values. The encoding
 * follows these rules:
 *
 * 1. **Single-byte representation (1 byte)**
 *    - If the integer is smaller than 2^6 (64), it is encoded in a single byte.
 *    - The two least significant bits (LSBs) of the byte are set to `0b00`.
 *    - The remaining 6 bits store the value.
 *
 * 2. **Two-byte representation (2 bytes)**
 *    - If the integer is between 2^6 (64) and 2^14 (16384), it is encoded in
 * two bytes.
 *    - The two LSBs of the first byte are set to `0b01`.
 *    - The remaining 14 bits store the value.
 *
 * 3. **Four-byte representation (4 bytes)**
 *    - If the integer is between 2^14 (16384) and 2^30 (1073741824), it is
 * encoded in four bytes.
 *    - The two LSBs of the first byte are set to `0b10`.
 *    - The remaining 30 bits store the value.
 *
 * 4. **Multibyte representation (variable length)**
 *    - If the integer is greater than 2^30 (1073741824), it is encoded using
 * multiple bytes.
 *    - The two LSBs of the first byte are set to `0b11`.
 *    - The remaining 6 bits of the first byte store the number of bytes
 * required minus 4.
 *    - The actual integer value is stored in little-endian order in the
 * following bytes.
 *
 * **Decoding follows the reverse process**:
 * - The two LSBs of the first byte determine the encoding format.
 * - The remaining bits and additional bytes (if needed) reconstruct the
 * original integer.
 *
 * This encoding ensures that smaller values occupy fewer bytes, leading to
 * efficient storage and transmission in SCALE-encoded data structures.
 */

#pragma once

#ifdef COMPACT_INTEGER_TYPE
static_assert(
    !"Already activated another type of CompactInteger: " COMPACT_INTEGER_TYPE);
#endif
#define COMPACT_INTEGER_TYPE "CLASSIC"

#include <scale/definitions.hpp>
#include <scale/detail/fixed_width_integer.hpp>
#include <scale/scale_error.hpp>
#include <scale/unreachable.hpp>

namespace scale {

  namespace detail::compact_integer {

    using fixed_width_integer::convert_to;

    /**
     * @brief Computes the number of significant bits required to represent an
     * integer.
     * @param integer The integer to analyze.
     * @return The number of significant bits.
     */
    size_t significant_bits(const CompactCompatible auto &integer) {
      if constexpr (std::integral<std::remove_cvref_t<decltype(integer)>>) {
        return std::bit_width(integer);
      } else {
        return integer.is_zero() ? 1 : msb(integer);
      }
    }

    /// Minimum integer value that requires 2 bytes encoding
    constexpr static size_t kMinUint16 = (1ul << 6u);
    /// Minimum integer value that requires 4 bytes encoding
    constexpr static size_t kMinUint32 = (1ul << 14u);
    /// Minimum integer value that requires multi-byte encoding
    constexpr static size_t kMinBigInteger = (1ul << 30u);

  }  // namespace detail::compact_integer

  /**
   * @brief Computes the length of compact encoding for a given integer.
   * @param val The integer to analyze.
   * @return The number of bytes required for encoding.
   */
  size_t lengthOfEncodedCompactInteger(CompactCompatible auto val) {
    if (val < detail::compact_integer::kMinUint16) return 1;
    if (val < detail::compact_integer::kMinUint32) return 2;
    if (val < detail::compact_integer::kMinBigInteger) return 4;
    if constexpr (std::unsigned_integral<decltype(val)>) {
      return 1 + std::bit_width(val);
    } else {
      return 1 + msb(val) / 8;
    }
  }

  /**
   * @brief Encodes an integer to compact representation using SCALE encoding.
   * @param integer Integer value to encode.
   * @param encoder SCALE encoder.
   */
  void encode(CompactInteger auto &&integer, Encoder &encoder) {
    if (integer < detail::compact_integer::kMinUint16) {
      uint8_t v = (convert_to<uint8_t>(integer) << 2u) | 0b00;
      return encoder.put(v);
    }

    if (integer < detail::compact_integer::kMinUint32) {
      uint16_t v = (convert_to<uint16_t>(integer) << 2u) | 0b01;
      return encode(v, encoder);
    }

    if (integer < detail::compact_integer::kMinBigInteger) {
      uint32_t v = (convert_to<uint32_t>(integer) << 2u) | 0b10;
      return encode(v, encoder);
    }

    const size_t significant_bytes_n =
        detail::compact_integer::significant_bits(untagged(integer)) / 8 + 1;
    if (significant_bytes_n > 67) {
      raise(EncodeError::VALUE_TOO_BIG_FOR_COMPACT_REPRESENTATION);
    }

    // The upper 6 bits of the header are used to encode the number of bytes
    // required to store the big integer. The value stored in these 6 bits
    // ranges from 0 to 63 (2^6 - 1). However, the actual byte count starts
    // from 4, so we subtract 4 from the byte count before encoding it.
    // This makes the range of byte counts for storing big integers 4 to 67.
    // To construct the final header, the upper 6 bits are shifted left by
    // 2 positions (equivalent to multiplying by 4).
    // The lower 2 bits (minor bits) store the encoding option, which in this
    // case is 0b11 (decimal value 3). The final header is formed by adding 3
    // to the result of the previous operations.
    uint8_t header = ((significant_bytes_n - 4) << 2u) | 0b11;
    encoder.put(header);

    for (auto v = integer; v != 0; v >>= 8) {
      encoder.put(convert_to<uint8_t>(v & 0xff));
    }
  }

  /**
   * @brief Decodes a compact-encoded integer from SCALE encoding.
   * @param integer The output integer.
   * @param decoder SCALE decoder.
   */
  void decode(CompactInteger auto &integer, Decoder &decoder) {
    using underlying_type =
        qtils::untagged_t<std::remove_cvref_t<decltype(integer)>>;

    auto adjust = [](CompactCompatible auto &&value) -> underlying_type {
      auto significant_bits = detail::compact_integer::significant_bits(value);
      constexpr auto target_bits = std::numeric_limits<underlying_type>::digits;
      if (significant_bits > target_bits) {
        raise(DecodeError::DECODED_VALUE_OVERFLOWS_TARGET);
      }
      return convert_to<underlying_type>(value);
    };

    auto first_byte = decoder.take();
    const uint8_t flag = first_byte & 0b00000011u;

    switch (flag) {
      case 0b00u: {
        auto value = static_cast<size_t>(first_byte >> 2u);
        integer = adjust(value);
        return;
      }

      case 0b01u: {
        auto second_byte = decoder.take();
        auto value = (static_cast<size_t>(first_byte & 0b11111100u)
                      + static_cast<size_t>(second_byte) * 256u)
                     >> 2u;
        if ((value >> 6) == 0) {
          raise(DecodeError::REDUNDANT_COMPACT_ENCODING);
        }
        integer = adjust(value);
        return;
      }

      case 0b10u: {
        size_t value = first_byte;
        size_t multiplier = 256u;
        if (not decoder.has(3)) {
          raise(DecodeError::NOT_ENOUGH_DATA);
        }
        for (auto i = 0u; i < 3u; ++i) {
          value += decoder.take() * multiplier;
          multiplier <<= 8u;
        }
        value = value >> 2u;
        if ((value >> 14) == 0) {
          raise(DecodeError::REDUNDANT_COMPACT_ENCODING);
        }
        integer = adjust(value);
        return;
      }

      case 0b11: {
        auto bytes_count = (first_byte >> 2u) + 4u;
        if (not decoder.has(bytes_count)) {
          raise(DecodeError::NOT_ENOUGH_DATA);
        }

        boost::multiprecision::uint1024_t multiplier{1u};
        boost::multiprecision::uint1024_t value{0};
        for (auto i = 0u; i < bytes_count; ++i) {
          value += decoder.take() * multiplier;
          multiplier <<= 8u;
        }
        if (value.is_zero()) {
          raise(DecodeError::REDUNDANT_COMPACT_ENCODING);
        }
        auto bits = msb(value) + 1;
        if (bits <= 30 or (bits + 7) / 8 < bytes_count) {
          raise(DecodeError::REDUNDANT_COMPACT_ENCODING);
        }
        integer = adjust(value);
        return;
      }
      default:
        UNREACHABLE;
    }
  }

}  // namespace scale
