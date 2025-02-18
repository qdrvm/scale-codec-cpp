/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Implements encoding and validation of enumeration values using SCALE
 * encoding.
 *
 * This file provides utilities for handling enumerations in SCALE encoding.
 * It allows defining constraints on enum values via `enum_traits`
 * specializations, ensuring that only valid values are encoded or decoded.
 * There are two ways to specialize an enumeration type:
 *
 * 1. **Defining a range of valid values** using `min_value` and `max_value`.
 * 2. **Providing an explicit list of valid values** using `valid_values`.
 *
 * The validation ensures that any decoded value belongs to the expected set,
 * reducing the risk of unexpected errors when processing SCALE-encoded data.
 */

#pragma once

#include <type_traits>

#include <scale/detail/tagged.hpp>
#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale {

  namespace detail::enumerations {

    template <typename T>
    concept Enumeration = std::is_enum_v<std::remove_cvref_t<T>>;

    /**
     * @brief Traits for enum validation.
     *
     * Provides two specialization choices:
     * - `min_value` and `max_value` convertible to `std::underlying_type_t<E>`.
     * - A container of `std::underlying_type_t<E>` named `valid_values`,
     * listing valid values.
     *
     * @note Check the macros below for specialization convenience.
     * @tparam E The enum type.
     */
    template <typename E>
      requires std::is_enum_v<E>
    struct [[deprecated(
        "Check the doc comment to see the specialization options")]]  //
    enum_traits final {
      /// Used to detect an unspecialized enum_traits
      static constexpr bool is_default = true;
    };

    /**
     * @brief Checks if a given value is within a defined range of valid enum
     * values.
     * @tparam T The input type (expected to be an enum or convertible
     * underlying type).
     * @param value The value to check.
     * @return True if the value is within the range, false otherwise.
     */
    template <typename T,
              typename E = std::decay_t<T>,
              typename E_traits = enum_traits<E>,
              std::underlying_type_t<E> Min = E_traits::min_value,
              std::underlying_type_t<E> Max = E_traits::max_value>
    constexpr bool is_valid_enum_value(
        std::underlying_type_t<E> value) noexcept {
      return value >= Min && value <= Max;
    }

    /**
     * @brief Checks if a given value is within an explicitly defined set of
     * valid enum values.
     * @tparam T The input type (expected to be an enum or convertible
     * underlying type).
     * @param value The value to check.
     * @return True if the value is listed in `valid_values`, false otherwise.
     */
    template <typename T,
              typename E = std::decay_t<T>,
              typename E_traits = enum_traits<E>,
              typename = decltype(E_traits::valid_values)>
    constexpr bool is_valid_enum_value(
        std::underlying_type_t<E> value) noexcept {
      const auto &valid_values = E_traits::valid_values;
      return std::find(std::begin(valid_values),
                       std::end(valid_values),
                       static_cast<E>(value))
             != std::end(valid_values);
    }

    /**
     * @brief Default case for unspecialized enum types.
     *
     * This function always returns `true`, but a `static_assert` ensures that
     * an explicit specialization of `enum_traits` is required.
     *
     * @tparam T The input type (expected to be an enum).
     * @return Always true, but triggers a compilation error if used.
     */
    template <typename T>
      requires enum_traits<std::decay_t<T>>::is_default
    [[deprecated(
        "Please specialize scale::enum_traits for your enum so it can be "
        "validated during decoding")]]
    constexpr bool is_valid_enum_value(
        std::underlying_type_t<std::decay_t<T>>) noexcept {
      return true;
    }
  }  // namespace detail::enumerations

  using detail::enumerations::enum_traits;
  using detail::enumerations::Enumeration;
  using detail::enumerations::is_valid_enum_value;

  /**
   * @brief Encodes an enumeration into its underlying type.
   * @param enumeration The enumeration value to encode.
   * @param encoder SCALE encoder.
   */
  void encode(const Enumeration auto &enumeration, Encoder &encoder)
    requires NoTagged<decltype(enumeration)>
  {
    using T =
        std::underlying_type_t<std::remove_cvref_t<decltype(enumeration)>>;
    encode(static_cast<T>(enumeration), encoder);
  }

  /**
   * @brief Decodes an enumeration from its underlying type.
   * @param v The enumeration value to decode into.
   * @param decoder SCALE decoder.
   */
  void decode(Enumeration auto &v, Decoder &decoder)
    requires NoTagged<decltype(v)>
  {
    using E = std::decay_t<decltype(v)>;
    std::underlying_type_t<E> value;
    decoder >> value;
    if (is_valid_enum_value<E>(value)) {
      v = static_cast<E>(value);
      return;
    }
    raise(DecodeError::INVALID_ENUM_VALUE);
  }

}  // namespace scale

/**
 * @brief Defines a valid range for an enumeration.
 * @note You should use this macro only in the global namespace
 * @param enum_namespace The namespace of the enum.
 * @param enum_name The enum type.
 * @param min The minimum valid value.
 * @param max The maximum valid value.
 */
#define SCALE_DEFINE_ENUM_VALUE_RANGE(enum_namespace, enum_name, min, max)  \
  template <>                                                               \
  struct scale::enum_traits<enum_namespace::enum_name> final {              \
    using underlying = std::underlying_type_t<enum_namespace::enum_name>;   \
    static constexpr underlying min_value = static_cast<underlying>((min)); \
    static constexpr underlying max_value = static_cast<underlying>((max)); \
  };

/**
 * @brief Defines a valid list of values for an enumeration.
 * @param enum_namespace The namespace of the enum.
 * @param enum_name The enum type.
 * @param ... The valid values.
 */
#define SCALE_DEFINE_ENUM_VALUE_LIST(enum_namespace, enum_name, ...) \
  template <>                                                        \
  struct scale::enum_traits<enum_namespace::enum_name> final {       \
    static constexpr bool is_default = false;                        \
    static constexpr std::array valid_values = {__VA_ARGS__};        \
  };
