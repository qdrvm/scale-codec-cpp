/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Implements encoding and validation of enumeration values using SCALE.
 *
 * This file provides utilities for handling enumerations in SCALE encoding.
 * It allows defining constraints on enum values via `enum_traits`
 * specializations, ensuring that only valid values are encoded or decoded.
 *
 * There are two ways to specialize an enumeration type:
 * - Define a range using `min_value` and `max_value`.
 * - Provide an explicit list using `valid_values`.
 *
 * Validation guarantees decoded values are within expected bounds,
 * reducing risk when handling SCALE-encoded data.
 */

#pragma once

#include <algorithm>
#include <limits>
#include <string_view>
#include <type_traits>

#include <scale/detail/tagged.hpp>
#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>

namespace scale {

  namespace detail::enumerations {

#define ENUM_NAME_PRETTY_FUNCTION __PRETTY_FUNCTION__
#if defined(__clang__)
    // clang-format off
    // Invalid:
    //   "std::string_view scale::detail::enumerations::enum_name_impl() [V = (Baz)-128]"
    // Valid:
    //   "std::string_view scale::detail::enumerations::enum_name_impl() [V = Enum_ValidatingByReflection_I8_Test::TestBody()::Baz::A]"
    // clang-format on
    constexpr std::string_view enum_prefix = "EnumValue = ";
    constexpr std::string_view enum_suffix = "]";
#elif defined(__GNUC__)
    // clang-format off
    // Invalid:
    //   "std::string_view scale::detail::enumerations::enum_name_impl() [with auto V = (Enum_ValidatingByReflection_I8_Test::TestBody::Baz)-128; std::string_view = std::basic_string_view<char>]"
    // Valid:
    //   "std::string_view scale::detail::enumerations::enum_name_impl() [with auto V = Enum_ValidatingByReflection_I8_Test::TestBody::Baz::A; std::string_view = std::basic_string_view<char>]"
    // clang-format on
    constexpr std::string_view enum_prefix = "EnumValue = ";
    constexpr std::string_view enum_suffix = "; ";
#else
#error Unsupported compiler
#endif

    /**
     * @brief Extracts enumeration name from the compiler-specific string.
     * @tparam V The enum value.
     */
    template <auto EnumValue>
    std::string_view enum_name_impl() {
      constexpr std::string_view func = ENUM_NAME_PRETTY_FUNCTION;
      constexpr std::size_t prefix_pos = func.find(enum_prefix);
      assert(prefix_pos != std::string_view::npos);
      constexpr std::size_t start = prefix_pos + enum_prefix.size();
      if (func[start] == '(') return {}; // Invalid, because wrapped by parenthesis)
      constexpr std::size_t end = func.find(enum_suffix, start);
      constexpr std::string_view full = func.substr(start, end - start);
      constexpr std::size_t colons = full.rfind("::");
      if (colons == std::string_view::npos) return {}; // Invalid, no namespace
      constexpr std::string_view result = full.substr(colons + 2);
      return result;
    }

    /**
     * @brief Concept that checks if a type is an enumeration.
     */
    template <typename T>
    concept Enumeration = std::is_enum_v<std::remove_cvref_t<T>>;

    /**
     * @brief Traits struct to be specialized for enumeration validation.
     */
    template <typename E>
    struct enum_traits;  // default not specialized

    namespace detail_impl {

      template <typename E>
      concept HasValidValues = requires { enum_traits<E>::valid_values; };

      template <typename E>
      concept HasMinMax = requires {
        enum_traits<E>::min_value;
        enum_traits<E>::max_value;
      };

      template <typename E, typename U = std::underlying_type_t<E>, U... Vs>
      constexpr bool is_valid_enum_value_by_reflection_impl(
          U value, std::integer_sequence<U, Vs...>) {
        return ((enum_name_impl<static_cast<E>(Vs)>().size() > 0
                 && static_cast<U>(static_cast<E>(Vs)) == value)
                || ...);
      }

      template <typename E, int... Is>
      constexpr bool call_with_casted_signed_range(
          std::underlying_type_t<E> value, std::integer_sequence<int, Is...>) {
        using U = std::underlying_type_t<E>;
        constexpr int min = -128;
        return is_valid_enum_value_by_reflection_impl<E>(
            value, std::integer_sequence<U, static_cast<U>(min + Is)...>{});
      }

      template <typename E, int... Is>
      constexpr bool call_with_casted_unsigned_range(
          std::underlying_type_t<E> value, std::integer_sequence<int, Is...>) {
        using U = std::underlying_type_t<E>;
        return is_valid_enum_value_by_reflection_impl<E>(
            value, std::integer_sequence<U, static_cast<U>(Is)...>{});
      }

      /**
       * @brief Fallback validation using reflection for enums of size 1 byte.
       */
      template <typename E>
        requires(sizeof(std::underlying_type_t<E>) == 1)
      constexpr bool is_valid_enum_value_by_reflection(
          std::underlying_type_t<E> value) {
        using U = std::underlying_type_t<E>;

        if constexpr (std::is_signed_v<U>) {
          constexpr int min = -128;
          constexpr int max = 127;
          return call_with_casted_signed_range<E>(
              value, std::make_integer_sequence<int, max - min + 1>{});
        } else {
          constexpr int max = 255;
          return call_with_casted_unsigned_range<E>(
              value, std::make_integer_sequence<int, max + 1>{});
        }
      }

      /**
       * @brief Validates enum value using min/max range.
       */
      template <typename T>
        requires HasMinMax<std::decay_t<T>>
      constexpr bool is_valid_enum_value_range(
          std::underlying_type_t<std::decay_t<T>> value) noexcept {
        using E = std::decay_t<T>;
        constexpr auto Min = enum_traits<E>::min_value;
        constexpr auto Max = enum_traits<E>::max_value;
        return value >= Min && value <= Max;
      }

      /**
       * @brief Validates enum value against list of valid values.
       */
      template <typename T>
        requires HasValidValues<std::decay_t<T>>
      constexpr bool is_valid_enum_value_list(
          std::underlying_type_t<std::decay_t<T>> value) noexcept {
        using E = std::decay_t<T>;
        const auto &valid_values = enum_traits<E>::valid_values;
        return std::find(valid_values.begin(),
                         valid_values.end(),
                         static_cast<E>(value))
               != valid_values.end();
      }

    }  // namespace detail_impl

    template <typename T>
    constexpr bool CannotValidateEnum = false;

    /**
     * @brief Central enum validation entry point.
     * @tparam T Enum type.
     * @param value The underlying integer value.
     * @return true if value is valid.
     */
    template <typename T>
      requires std::is_enum_v<std::decay_t<T>>
    constexpr bool is_valid_enum_value(
        std::underlying_type_t<std::decay_t<T>> value) noexcept {
      using E = std::decay_t<T>;

      if constexpr (detail_impl::HasValidValues<E>) {
        return detail_impl::is_valid_enum_value_list<T>(value);
      } else if constexpr (detail_impl::HasMinMax<E>) {
        return detail_impl::is_valid_enum_value_range<T>(value);
      } else if constexpr (sizeof(std::underlying_type_t<E>) == 1) {
        return detail_impl::is_valid_enum_value_by_reflection<E>(value);
      } else {
        static_assert(CannotValidateEnum<T>,
                      "Cannot validate enum: "
                      "define enum_traits<> with min/max or valid_values.");
        return true;
      }
    }

  }  // namespace detail::enumerations

  using detail::enumerations::enum_traits;
  using detail::enumerations::Enumeration;
  using detail::enumerations::is_valid_enum_value;

  /**
   * @brief Encodes an enum value using SCALE encoding.
   * @tparam T Enum type.
   * @param enumeration Enum value to encode.
   * @param encoder Encoder instance.
   */
  void encode(const Enumeration auto &enumeration, Encoder &encoder)
    requires NoTagged<decltype(enumeration)>
  {
    using T =
        std::underlying_type_t<std::remove_cvref_t<decltype(enumeration)>>;
    encode(static_cast<T>(enumeration), encoder);
  }

  /**
   * @brief Decodes an enum value using SCALE decoding.
   * @tparam T Enum type.
   * @param v Enum variable to store the decoded value.
   * @param decoder Decoder instance.
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
 * @def SCALE_DEFINE_ENUM_VALUE_RANGE
 * @brief Defines a valid value range for an enum.
 * @param enum_namespace Namespace where enum is defined.
 * @param enum_name Enum type name.
 * @param min Minimum valid value.
 * @param max Maximum valid value.
 */
#define SCALE_DEFINE_ENUM_VALUE_RANGE(enum_namespace, enum_name, min, max)  \
  template <>                                                               \
  struct scale::enum_traits<enum_namespace::enum_name> final {              \
    using underlying = std::underlying_type_t<enum_namespace::enum_name>;   \
    static constexpr underlying min_value = static_cast<underlying>((min)); \
    static constexpr underlying max_value = static_cast<underlying>((max)); \
  };

/**
 * @def SCALE_DEFINE_ENUM_VALUE_LIST
 * @brief Defines an explicit list of valid enum values.
 * @param enum_namespace Namespace where enum is defined.
 * @param enum_name Enum type name.
 * @param ... List of valid values.
 */
#define SCALE_DEFINE_ENUM_VALUE_LIST(enum_namespace, enum_name, ...) \
  template <>                                                        \
  struct scale::enum_traits<enum_namespace::enum_name> final {       \
    static constexpr bool is_default = false;                        \
    static constexpr std::array valid_values = {__VA_ARGS__};        \
  };
