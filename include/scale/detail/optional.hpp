/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Provides encoding and decoding functions for optional types in SCALE
 * codec.
 *
 * This file defines utilities for handling both `std::optional` and
 * `boost::optional` in SCALE encoding and decoding processes. Special handling
 * is provided for `optional<bool>` to optimize encoding space.
 */

#pragma once

#include <optional>

#ifdef __has_include
#if __has_include(<boost/optional.hpp>)
#include <boost/optional.hpp>
#define HAS_BOOST_OPTIONAL
#endif
#endif

#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale {

  namespace detail::optional {
    /**
     * @brief Helper struct to check if a type is `std::optional`.
     *
     * @tparam T The type to check.
     */
    template <typename>
    struct is_std_optional_impl : std::false_type {};

    template <typename T>
    struct is_std_optional_impl<std::optional<T>> : std::true_type {};

    /**
     * @brief Determines whether a type is `std::optional`.
     *
     * @tparam T The type to check.
     */
    template <typename T>
    constexpr bool is_std_optional =
        is_std_optional_impl<std::remove_cvref_t<T>>::value;

    /**
     * @brief Concept for `std::optional`.
     */
    template <typename T>
    concept StdOptional = is_std_optional<T>;

    /**
     * @brief Helper struct to check if a type is `boost::optional`.
     *
     * @tparam T The type to check.
     */
    template <typename>
    struct is_boost_optional_impl : std::false_type {};

#ifdef HAS_BOOST_OPTIONAL
    template <typename T>
    struct is_boost_optional_impl<boost::optional<T>> : std::true_type {};
#endif

    /**
     * @brief Determines whether a type is `boost::optional`.
     *
     * @tparam T The type to check.
     */
    template <typename T>
    constexpr bool is_boost_optional =
        is_boost_optional_impl<std::remove_cvref_t<T>>::value;

    /**
     * @brief Extracts the value type from an optional type.
     *
     * @tparam T An optional type.
     */
    template <typename T>
      requires is_std_optional<T> or is_boost_optional<T>
    using optional_value_type =
        std::remove_cvref_t<decltype(*std::declval<T>())>;

    /**
     * @brief Concept for `optional<bool>`.
     */
    template <typename T>
    concept OptionalBool = std::same_as<optional_value_type<T>, bool>;

    /**
     * @brief Concept for optional types excluding `optional<bool>`.
     */
    template <typename T>
    concept Optional =
        (is_std_optional<T> or is_boost_optional<T>) and not OptionalBool<T>;

  }  // namespace detail::optional

  using detail::optional::Optional;
  using detail::optional::OptionalBool;

  /**
   * @brief Encodes an `optional<bool>` value into SCALE format.
   *
   * @param opt_bool The `optional<bool>` to encode.
   * @param encoder The SCALE encoder.
   */
  void encode(OptionalBool auto &&opt_bool, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(opt_bool)>
  {
    if (opt_bool.has_value()) {
      encoder.put(static_cast<uint8_t>(opt_bool.value() ? 1  // True
                                                        : 2  // False
                                       ));
    } else {
      encoder.put(0);  // None
    }
  }

  /**
   * @brief Encodes a general optional value into SCALE format.
   *
   * @param optional The optional value to encode.
   * @param encoder The SCALE encoder.
   */
  void encode(Optional auto &&optional, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(optional)>
  {
    if (not optional.has_value()) {
      encoder.put(0);
    } else {
      encoder.put(1);
      encode(optional.value(), encoder);
    }
  }

  void encode(const std::nullopt_t &, ScaleEncoder auto &encoder) {
    encoder.put(0);
  }

  /**
   * @brief Decodes an `optional<bool>` value from SCALE format.
   *
   * @param opt_bool The `optional<bool>` to decode.
   * @param decoder The SCALE decoder.
   */
  void decode(OptionalBool auto &&opt_bool, ScaleDecoder auto &decoder)
    requires NoTagged<decltype(opt_bool)>
  {
    auto byte = decoder.take();
    switch (byte) {
      case 0:
        opt_bool.reset();
        return;
      case 1:
        opt_bool = true;
        return;
      case 2:
        opt_bool = false;
        return;
      default:
        raise(DecodeError::UNEXPECTED_VALUE);
    }
  }

  /**
   * @brief Decodes a general optional value from SCALE format.
   *
   * @param optional The optional value to decode.
   * @param decoder The SCALE decoder.
   */
  void decode(Optional auto &&optional, ScaleDecoder auto &decoder)
    requires NoTagged<decltype(optional)>
  {
    auto byte = decoder.take();
    switch (byte) {
      case 0:
        optional.reset();
        return;
      case 1:
        optional.emplace();
        decode(optional.value(), decoder);
        return;
      default:
        raise(DecodeError::UNEXPECTED_VALUE);
    }
  }

}  // namespace scale