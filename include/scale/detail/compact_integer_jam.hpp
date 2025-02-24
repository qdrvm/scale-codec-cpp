/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Implements encoding and decoding of compact integers using JAM
 * encoding.
 *
 * The JAM encoding scheme represents integers in a variable-length format.
 * It uses a prefix byte to store the least significant bits of the value and
 * encodes the remaining bits in subsequent bytes. The first bit of each byte
 * determines whether more bytes are required: if set, additional bytes follow;
 * otherwise, decoding stops.
 *
 * This allows efficient encoding of small numbers while also supporting large
 * integers with minimal overhead.
 */

#pragma once

#ifdef COMPACT_INTEGER_TYPE
static_assert(
    !"Already activated another type of CompactInteger: " COMPACT_INTEGER_TYPE);
#endif
#define COMPACT_INTEGER_TYPE "JAM"

#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>

namespace scale {

  /**
   * @brief Computes the length of the encoded compact integer.
   * @param value The integer to analyze.
   * @return The number of bytes required for encoding.
   */
  size_t lengthOfEncodedCompactInteger(CompactCompatible auto &&value) {
    using underlying_type =
        qtils::untagged_t<std::remove_cvref_t<decltype(value)>>;
    if constexpr (std::unsigned_integral<underlying_type>) {
      return 1 + (std::bit_width(value) - 1) / 7;
    } else {
      return 1 + msb(value - 1) / 7;
    }
  }

  /**
   * @brief Encodes an integer to JAM compact representation using SCALE
   * encoding.
   * @param integer Integer value to encode.
   * @param encoder SCALE encoder.
   */
  void encode(CompactInteger auto &&integer, Encoder &encoder) {
    using underlying_type =
        qtils::untagged_t<std::remove_cvref_t<decltype(integer)>>;

    size_t value;

    if constexpr (std::unsigned_integral<underlying_type>) {
      value = static_cast<size_t>(integer);
    } else {
      if (integer.is_zero()) {
        value = 0;
      } else {
        if (msb(integer) >= std::numeric_limits<size_t>::digits) {
          raise(EncodeError::VALUE_TOO_BIG_FOR_COMPACT_REPRESENTATION);
        }
        value = integer.template convert_to<size_t>();
      }
    }

    if (value < 0x80) {
      encoder.put(static_cast<uint8_t>(value));
      return;
    }

    // NOLINTNEXTLINE(*-pro-type-member-init)
    std::array<uint8_t, sizeof(size_t) + 1> bytes;
    uint8_t &prefix = bytes[0];
    prefix = 0;
    size_t len = 1;

    for (decltype(value) i = value; i != 0; i >>= 8) {
      // minimal value in prefix
      if (i <= (static_cast<uint8_t>(~prefix) >> 1)) {
        prefix |= i;
        break;
      }
      prefix = (prefix >> 1) | 0x80;
      bytes[len++] = static_cast<uint8_t>(i & 0xff);
    }

    encoder.write({bytes.data(), len});
  }

  /**
   * @brief Decodes a JAM compact-encoded integer from SCALE encoding.
   * @param integer The output integer.
   * @param decoder SCALE decoder.
   */
  void decode(CompactInteger auto &integer, Decoder &decoder) {
    using underlying_type =
        qtils::untagged_t<std::remove_cvref_t<decltype(integer)>>;

    uint8_t byte = decoder.take();

    if (byte == 0) {
      integer = 0;
      return;
    }

    uint8_t len_bits = byte;
    uint8_t val_bits = byte;
    uint8_t val_mask = 0xff;

    size_t value = 0;

    for (uint8_t i = 0; i < 8; ++i) {
      val_mask >>= 1;
      val_bits &= val_mask;

      // check if no more significant bytes
      if ((len_bits & static_cast<uint8_t>(0x80)) == 0) {
        value |= static_cast<size_t>(val_bits) << (8 * i);
        break;
      }
      len_bits <<= 1;

      byte = decoder.take();
      value |= static_cast<size_t>(byte) << (8 * i);
    }
    if (val_bits == 0 and (byte & ~val_mask) == 0) {
      raise(DecodeError::REDUNDANT_COMPACT_ENCODING);
    }

    if (value > std::numeric_limits<underlying_type>::max()) {
      raise(DecodeError::DECODED_VALUE_OVERFLOWS_TARGET);
    }

    integer = value;
  }

}  // namespace scale
