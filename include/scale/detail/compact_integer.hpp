/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines utilities for handling compact integer types in SCALE
 * serialization.
 *
 * This file provides support for compact integer types, including tagging,
 * conversion, and encoding/decoding mechanisms.
 */

#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <qtils/tagged.hpp>

#include <scale/definitions.hpp>
#include <scale/types.hpp>

namespace scale {

  namespace detail::compact_integer {

    /**
     * @brief Tag used for identifying compact integers.
     */
    struct CompactIntegerTag;

    /**
     * @brief Determines if a multiprecision backend is unsigned.
     * @tparam Backend The multiprecision backend type.
     */
    template <typename Backend>
    constexpr bool is_unsigned_backend = not Backend().sign();

    /**
     * @brief Concept for checking if a type is an unsigned multiprecision
     * integer.
     * @tparam T The type to check.
     */
    template <typename T>
    concept unsigned_multiprecision_integer =
        boost::multiprecision::is_unsigned_number<T>::value;

    /**
     * @brief Trait to check if a type is a compact integer.
     * @tparam T The type to check.
     */
    template <typename T>
    struct is_compact_integer : std::false_type {};

    template <typename T>
    struct is_compact_integer<qtils::Tagged<T, CompactIntegerTag>>
        : std::true_type {};

    /**
     * @brief Concept defining a valid Compact type.
     * @tparam T The type to check.
     */
    template <typename T>
    concept CompactCompatible =
        std::unsigned_integral<std::remove_cvref_t<T>>
        or unsigned_multiprecision_integer<std::remove_cvref_t<T>>;

    /**
     * @brief Represents a compact integer value.
     * @tparam T The underlying type.
     */
    template <typename T>
      requires CompactCompatible<T>
    using Compact = qtils::Tagged<T, CompactIntegerTag>;

    /**
     * @brief Concept that checks if a type is a Compact integer.
     * @tparam T The type to check.
     */
    template <typename T>
    concept CompactInteger = is_compact_integer<std::remove_cvref_t<T>>::value;

    /**
     * @brief Converts a CompactInteger to another type.
     * @tparam T The target type.
     * @param value The compact integer to convert.
     * @return Converted value.
     */
    template <typename T>
    T convert_to(CompactInteger auto &&value) {
      using underlying_type =
          qtils::untagged_t<std::remove_cvref_t<decltype(value)>>;
      if constexpr (std::unsigned_integral<underlying_type>) {
        return static_cast<T>(value);
      } else {
        return value.template convert_to<T>();
      }
    }

    /**
     * @brief Reflection structure for Compact types.
     * @tparam T The Compact-compatible type.
     */
    template <typename T>
      requires CompactCompatible<std::remove_cvref_t<T>>
    struct CompactReflection {
     private:
      template <typename U>
      explicit CompactReflection(U &&value)
          : temp_storage(
                std::is_lvalue_reference_v<U>
                    ? std::nullopt
                    : std::optional<std::decay_t<U>>(std::forward<U>(value))),
            ref(temp_storage.has_value() ? temp_storage.value() : value) {}

      template <typename U>
      friend decltype(auto) as_compact(U &&value);

     public:
      CompactReflection(const CompactReflection &) = delete;
      CompactReflection &operator=(const CompactReflection &) = delete;
      CompactReflection(CompactReflection &&) = delete;
      CompactReflection &operator=(CompactReflection &&) = delete;

      /**
       * @brief Encodes a CompactReflection value.
       * @param value The CompactReflection instance.
       * @param encoder The encoder to use.
       */
      friend void encode(const CompactReflection &value, Encoder &encoder) {
        encode(Compact<std::remove_cvref_t<T>>(value.ref), encoder);
      }

      /**
       * @brief Decodes a CompactReflection value.
       * @param value The CompactReflection instance.
       * @param decoder The decoder to use.
       */
      friend void decode(CompactReflection &value, Decoder &decoder) {
        Compact<std::remove_cvref_t<T>> tmp;
        decode(tmp, decoder);
        value.ref = untagged(tmp);
      }

     private:
      std::optional<std::decay_t<T>> temp_storage;
      T &ref;
    };

    /**
     * @brief Wraps a value in a CompactReflection structure.
     * @tparam T The Compact-compatible type.
     * @param value The value to wrap.
     * @return CompactReflection instance.
     */
    template <typename T>
    decltype(auto) as_compact(T &&value) {
      return CompactReflection<decltype(value)>(
          std::forward<decltype(value)>(value));
    }
  }  // namespace detail::compact_integer

  using detail::compact_integer::as_compact;
  using detail::compact_integer::Compact;
  using detail::compact_integer::CompactCompatible;
  using detail::compact_integer::CompactInteger;
}  // namespace scale

#ifdef JAM_COMPATIBILITY_ENABLED
#include <scale/detail/compact_integer_jam.hpp>
#else
#include <scale/detail/compact_integer_classic.hpp>
#endif
