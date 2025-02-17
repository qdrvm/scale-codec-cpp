/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <type_traits>

#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>

namespace scale {

  class ScaleDecoderStream;

  /**
   * Description of an enum type
   * Two specialization choices:
   *  - min_value and max_value convertible to std::underlying_type_t<E>
   *  - a container of std::underlying_type_t<E> named valid_values, listing
   *    valid values
   * @note check macros below for specialization convenience
   * @tparam E the enum type
   */
  template <typename E>
    requires std::is_enum_v<E>
  struct [[deprecated(
      "Check the doc comment to see the specialization options")]]  //
  enum_traits final {
    // Used to easily detect an unspecialized enum_traits
    static constexpr bool is_default = true;
  };

#define SCALE_DEFINE_ENUM_VALUE_RANGE(enum_namespace, enum_name, min, max)  \
  template <>                                                               \
  struct scale::enum_traits<enum_namespace::enum_name> final {              \
    using underlying = std::underlying_type_t<enum_namespace::enum_name>;   \
    static constexpr underlying min_value = static_cast<underlying>((min)); \
    static constexpr underlying max_value = static_cast<underlying>((max)); \
  };

// Mind that values should be enum constants, not numbers
#define SCALE_DEFINE_ENUM_VALUE_LIST(enum_namespace, enum_name, ...)           \
  template <>                                                                  \
  struct scale::enum_traits<enum_namespace::enum_name> final {                 \
    static constexpr enum_namespace::enum_name valid_values[] = {__VA_ARGS__}; \
  };

  template <typename T,
            typename E = std::decay_t<T>,
            typename E_traits = enum_traits<E>,
            std::underlying_type_t<E> Min = E_traits::min_value,
            std::underlying_type_t<E> Max = E_traits::max_value>
  constexpr bool is_valid_enum_value(std::underlying_type_t<E> value) noexcept {
    return value >= Min && value <= Max;
  }

  template <typename T,
            typename E = std::decay_t<T>,
            typename E_traits = enum_traits<E>,
            typename = decltype(E_traits::valid_values)>
  constexpr bool is_valid_enum_value(std::underlying_type_t<E> value) noexcept {
    const auto &valid_values = E_traits::valid_values;
    return std::ranges::find(valid_values, static_cast<E>(value))
           != std::end(valid_values);
  }

  template <typename T>
    requires enum_traits<std::decay_t<T>>::is_default
  [[deprecated(
      "Please specialize scale::enum_traits for your enum so it can be "
      "validated during decoding")]]
  constexpr bool is_valid_enum_value(
      std::underlying_type_t<std::decay_t<T>> value) noexcept {
    return true;
  }

  /**
   * @brief scale-decodes any enum type as underlying type
   * @tparam T enum type
   * @param v value of enum type
   * @return reference to stream
   */
  template <typename T>
    requires std::is_enum_v<std::remove_cvref_t<T>>
  ScaleDecoderStream &operator>>(ScaleDecoderStream &s, T &v) {
    using E = std::decay_t<T>;
    std::underlying_type_t<E> value;
    s >> value;
    if (is_valid_enum_value<E>(value)) {
      v = static_cast<E>(value);
      return s;
    }
    raise(DecodeError::INVALID_ENUM_VALUE);
  }

}  // namespace scale
