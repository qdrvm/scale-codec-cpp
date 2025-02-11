/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <qtils/tagged.hpp>

#include <scale/definitions.hpp>
#include <scale/types.hpp>

namespace scale {

  namespace detail::compact_integer {

    struct CompactIntegerTag;

    template <typename Backend>
    constexpr bool is_unsigned_backend = not Backend().sign();

    template <typename T>
    concept unsigned_multiprecision_integer =
        boost::multiprecision::is_unsigned_number<T>::value;

    template <typename>
    struct is_compact_integer : std::false_type {};

    template <typename T>
    struct is_compact_integer<qtils::Tagged<T, CompactIntegerTag>>
        : std::true_type {};

    /// @brief Concept defining a valid Compact type
    template <typename T>
    concept CompactCompatible =
        std::unsigned_integral<std::remove_cvref_t<T>>
        or unsigned_multiprecision_integer<std::remove_cvref_t<T>>;

    /// @brief Represents a compact integer value
    template <typename T>
      requires CompactCompatible<T>
    using Compact = qtils::Tagged<T, CompactIntegerTag>;

    /// @brief Concept that checks if a type is a Compact integer
    template <typename T>
    concept CompactInteger = is_compact_integer<std::remove_cvref_t<T>>::value;

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
        requires CompactCompatible<std::remove_cvref_t<U>>
      friend decltype(auto) as_compact(U &&value);

     public:
      CompactReflection(const CompactReflection &) = delete;
      CompactReflection &operator=(const CompactReflection &) = delete;
      CompactReflection(CompactReflection &&) = delete;
      CompactReflection &operator=(CompactReflection &&) = delete;

      friend void encode(const CompactReflection &value,
                         ScaleEncoder auto &encoder) {
        encode(Compact<std::remove_cvref_t<T>>(value.ref), encoder);
      }
      friend void decode(const CompactReflection &value,
                         ScaleDecoder auto &decoder) {
        Compact<std::remove_cvref_t<T>> tmp;
        decode(tmp, decoder);
        value.ref = untagged(tmp);
      }

     private:
      std::optional<std::decay_t<T>> temp_storage;
      T &ref;
    };

    template <typename T>
      requires CompactCompatible<std::remove_cvref_t<T>>
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
