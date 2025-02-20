/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Provides encoding and decoding functions for fixed-width and
 * multiprecision integers using SCALE encoding. in SCALE encoding/decoding.
 */

#pragma once

#include <boost/endian/buffers.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <scale/detail/tagged.hpp>
#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale {

  /// Type aliases for multiprecision integers
  using uint128_t = boost::multiprecision::uint128_t;
  using uint256_t = boost::multiprecision::uint256_t;
  using uint512_t = boost::multiprecision::uint512_t;
  using uint1024_t = boost::multiprecision::uint1024_t;
  using int128_t = boost::multiprecision::int128_t;
  using int256_t = boost::multiprecision::int256_t;
  using int512_t = boost::multiprecision::int512_t;
  using int1024_t = boost::multiprecision::int1024_t;

  namespace detail::fixed_width_integer {

    /// Concept to check if a type is a small integer (excluding bool)
    template <typename T>
    constexpr bool is_small_integer =
        std::is_integral_v<T> and not std::is_same_v<T, bool>;

    /// Concept to check if a type is a big integer (boost multiprecision)
    template <typename T>
    concept is_big_integer =
        std::is_same_v<T, uint128_t> or std::is_same_v<T, uint256_t>
        or std::is_same_v<T, uint512_t> or std::is_same_v<T, uint1024_t>
        or std::is_same_v<T, int128_t> or std::is_same_v<T, int256_t>
        or std::is_same_v<T, int512_t> or std::is_same_v<T, int1024_t>;

    /// Concept alias for small integer type
    template <typename T>
    concept SmallInteger = is_small_integer<std::remove_cvref_t<T>>;

    /// Concept alias for big integer type
    template <typename T>
    concept BigInteger = is_big_integer<std::remove_cvref_t<T>>;

    /// Concept alias for fixed-width integer type (either small or big integer)
    template <typename T>
    concept FixedInteger = SmallInteger<T> or BigInteger<T>;

    /**
     * @brief Converts a value from one type to another using static_cast.
     * @tparam T Target type.
     * @tparam From Source type.
     * @param value Value to convert.
     * @return Converted value.
     */
    template <typename T>
    T convert_to(SmallInteger auto &&value)
      requires std::is_convertible_v<std::remove_cvref_t<decltype(value)>, T>
    {
      return static_cast<T>(value);
    }

    /**
     * @brief Specialized conversion for Boost multiprecision numbers.
     * @tparam T Target type.
     * @param value Value to convert.
     * @return Converted value.
     * @throws std::system_error if conversion results in data loss.
     */
    template <typename T>
    T convert_to(BigInteger auto &&value) {
      try {
        return value.template convert_to<T>();
      } catch (const std::runtime_error &) {
        throw std::system_error{
            make_error_code(std::errc::value_too_large),
            "This integer conversion would lead to information loss"};
      }
    }

  }  // namespace detail::fixed_width_integer

  /// Expose some concepts and functions
  using detail::fixed_width_integer::BigInteger;
  using detail::fixed_width_integer::convert_to;
  using detail::fixed_width_integer::FixedInteger;
  using detail::fixed_width_integer::SmallInteger;

  /**
   * @brief Encodes a boolean value into SCALE encoding.
   * @param boolean Boolean value to encode.
   * @param encoder SCALE encoder.
   */
  inline void encode(bool boolean, Encoder &encoder) {
    encoder.put(boolean ? 1u : 0u);
  }

  /**
   * @brief Decodes a boolean value from SCALE encoding.
   * @param boolean Boolean value to decode.
   * @param decoder SCALE decoder.
   */
  inline void decode(bool &boolean, Decoder &decoder) {
    switch (auto byte = decoder.take()) {
      case 0u:
        boolean = false;
        return;
      case 1u:
        boolean = true;
        return;
      default:
        raise(DecodeError::UNEXPECTED_VALUE);
    }
  }

  /**
   * @brief Encodes a small integer into little-endian representation.
   * @param integer Integer value to encode.
   * @param encoder SCALE encoder.
   */
  void encode(const SmallInteger auto &integer, Encoder &encoder)
    requires NoTagged<decltype(integer)>
  {
    using Integer = std::remove_cvref_t<decltype(integer)>;
    if constexpr (sizeof(Integer) == 1u) {
      return encoder.put(static_cast<uint8_t>(integer));
    }
    constexpr size_t size = sizeof(integer);
    constexpr size_t bits = size * 8;
    boost::endian::endian_buffer<boost::endian::order::little, Integer, bits>
        buf;
    buf = integer;
    encoder.write({buf.data(), size});
  }

  /**
   * @brief Encodes a big integer using SCALE encoding.
   * @param integer Big integer to encode.
   * @param encoder SCALE encoder.
   */
  void encode(BigInteger auto &&integer, Encoder &encoder)
    requires NoTagged<decltype(integer)>
  {
    constexpr auto size =
        (std::numeric_limits<std::decay_t<decltype(integer)>>::digits + 1) / 8;
    // NOLINTNEXTLINE(*-pro-type-member-init)
    std::array<uint8_t, size> buff{};
    export_bits(integer, buff.data(), 8, false);
    encoder.write(std::span(buff.data(), size));
  }

  /**
   * @brief Decodes a small integer from SCALE encoding.
   * @param integer Small integer to decode.
   * @param decoder SCALE decoder.
   */
  void decode(SmallInteger auto &integer, Decoder &decoder) {
    using Integer = std::remove_cvref_t<decltype(integer)>;
    if constexpr (sizeof(Integer) == 1u) {
      integer = decoder.take();
      return;
    }
    if (not decoder.has(sizeof(Integer))) {
      raise(DecodeError::NOT_ENOUGH_DATA);
    }
    auto data = decoder.read(sizeof(Integer)).data();
    integer = boost::endian::endian_load<Integer,
                                         sizeof(Integer),
                                         boost::endian::order::little>(data);
  }

  /**
   * @brief Decodes a big integer from SCALE encoding.
   * @param integer Big integer to decode.
   * @param decoder SCALE decoder.
   */
  void decode(BigInteger auto &integer, Decoder &decoder) {
    constexpr auto size =
        (std::numeric_limits<std::decay_t<decltype(integer)>>::digits + 1) / 8;
    auto slice = decoder.read(size);
    import_bits(integer, slice.begin(), slice.end(), 8, false);
  }

}  // namespace scale