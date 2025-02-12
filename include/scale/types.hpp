/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

#include <boost/multiprecision/cpp_int.hpp>
#include <qtils/tagged.hpp>

#include <scale/definitions.hpp>

namespace scale {

  class ScaleEncoderStream;
  class ScaleDecoderStream;

  using uint128_t = boost::multiprecision::uint128_t;
  using uint256_t = boost::multiprecision::uint256_t;
  using uint512_t = boost::multiprecision::uint512_t;
  using uint1024_t = boost::multiprecision::uint1024_t;

  /// @brief convenience alias for arrays of bytes
  using ByteArray = std::vector<uint8_t>;

  /// @brief convenience alias for immutable span of bytes
  using ConstSpanOfBytes = std::span<const uint8_t>;

  /// @brief convenience alias for mutable span of bytes
  using MutSpanOfBytes = std::span<uint8_t>;

  namespace detail {
    struct CompactIntegerTag;

    template <typename Backend>
    constexpr bool is_unsigned_backend = not Backend().sign();

    template <typename T>
    concept unsigned_multiprecision_integer =
        boost::multiprecision::is_unsigned_number<T>::value;

    template <typename T>
    struct is_compact_integer : std::false_type {};

    template <typename T>
    struct is_compact_integer<qtils::Tagged<T, CompactIntegerTag>>
        : std::true_type {};
  }  // namespace detail

  /// @brief Concept defining a valid Compact type
  template <typename T>
  concept CompactCompatible =
      std::unsigned_integral<T> or detail::unsigned_multiprecision_integer<T>;

  /// @brief Represents a compact integer value
  template <typename T>
    requires CompactCompatible<T>
  using Compact = qtils::Tagged<T, detail::CompactIntegerTag>;

  /// @brief Concept that checks if a type is a Compact integer
  template <typename T>
  concept CompactInteger =
      detail::is_compact_integer<std::remove_cvref_t<T>>::value;

  using Length = Compact<size_t>;

  template <typename T>
  inline T convert_to(const CompactCompatible auto &&value) {
    constexpr auto is_integral =
        std::unsigned_integral<std::remove_cvref_t<decltype(value)>>;
    if constexpr (is_integral) {
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
          ref(temp_storage ? *temp_storage : value) {}

    template <typename U>
    friend decltype(auto) as_compact(U &&value);

   public:
    CompactReflection(const CompactReflection &) = delete;
    CompactReflection &operator=(const CompactReflection &) = delete;
    CompactReflection(CompactReflection &&) = delete;
    CompactReflection &operator=(CompactReflection &&) = delete;

    friend ScaleEncoderStream &operator<<(ScaleEncoderStream &stream,
                                          const CompactReflection &value) {
      return stream << Compact<std::remove_cvref_t<T>>(value.ref);
    }
    friend ScaleDecoderStream &operator>>(ScaleDecoderStream &stream,
                                          const CompactReflection &value) {
      Compact<std::remove_cvref_t<T>> tmp;
      stream >> tmp;
      value.ref = untagged(tmp);
      return stream;
    }

   private:
    std::optional<std::decay_t<T>> temp_storage;
    T &ref;
  };

  template <typename T>
  decltype(auto) as_compact(T &&value) {
    return CompactReflection<decltype(value)>(
        std::forward<decltype(value)>(value));
  }

  /// @brief OptionalBool is internal extended bool type
  enum class OptionalBool : uint8_t {
    NONE = 0u,
    OPT_TRUE = 1u,
    OPT_FALSE = 2u,
  };

  namespace detail {
    struct ArgHelper {
      template <typename T>
      operator T() const {
        return T{};
      }
    };

    template <typename T, std::size_t... Indices>
      requires std::is_aggregate_v<T>
    constexpr bool is_constructible_with_n_def_args_impl(
        std::index_sequence<Indices...>) {
      return requires { T{(void(Indices), ArgHelper{})...}; };
    }

    template <typename T, size_t N>
    constexpr bool is_constructible_with_n_def_args_v =
        is_constructible_with_n_def_args_impl<T>(std::make_index_sequence<N>{});

    template <typename T, int N = 0>
    constexpr int field_number_of_impl() {
      if constexpr (std::is_empty_v<T>) {
        return 0;
      } else if constexpr (is_constructible_with_n_def_args_v<T, N + 1>) {
        return field_number_of_impl<T, N + 1>();
      } else {
        return N;
      }
    }

    template <typename T>
    constexpr size_t field_number_of =
        field_number_of_impl<std::remove_cvref_t<T>>();

    template <typename T>
    concept is_std_array =
        requires { typename std::tuple_size<std::remove_cvref_t<T>>::type; }
        and std::same_as<
            std::remove_cvref_t<T>,
            std::array<typename std::remove_cvref_t<T>::value_type,
                       std::tuple_size<std::remove_cvref_t<T>>::value>>;
  }  // namespace detail

  template <typename T>
  concept SomeSpan =
      std::derived_from<T, std::span<typename T::element_type, T::extent>>;

  template <typename T>
  concept HasSomeInsertMethod = requires(T v) {
    v.insert(v.end(), *v.begin());
  } or requires(T v) { v.insert_after(v.end(), *v.begin()); };

  template <typename T>
  concept HasResizeMethod = requires(T v) { v.resize(v.size()); };

  template <typename T>
  concept HasReserveMethod = requires(T v) { v.reserve(v.size()); };

  template <typename T>
  concept HasEmplaceMethod = requires(T v) { v.emplace(*v.begin()); };

  template <typename T>
  concept HasEmplaceBackMethod = requires(T v) { v.emplace_back(*v.begin()); };

  template <typename T>
  concept ImplicitlyDefinedAsStatic = not(SomeSpan<T>) and  //
                                      not(HasSomeInsertMethod<T>);

  template <typename T>
  concept ImplicitlyDefinedAsDynamic = not(SomeSpan<T>) and  //
                                       HasSomeInsertMethod<T>;

  template <typename T>
  concept StaticSpan = SomeSpan<T>  //
                       and (T::extent != std::dynamic_extent);

  template <typename T>
  concept DynamicSpan = SomeSpan<T>  //
                        and (T::extent == std::dynamic_extent);

  template <typename T>
  concept StaticCollection = std::ranges::range<T>
                             and (ImplicitlyDefinedAsStatic<T>  //
                                  or StaticSpan<T>);

  template <typename T>
  concept DynamicCollection = std::ranges::sized_range<T>
                              and (ImplicitlyDefinedAsDynamic<T>  //
                                   or DynamicSpan<T>);

  template <typename T>
  concept ResizeableCollection = DynamicCollection<T>  //
                                 and HasResizeMethod<T>;

  template <typename T>
  concept ExtensibleBackCollection = DynamicCollection<T>         //
                                     and not(HasResizeMethod<T>)  //
                                     and HasEmplaceBackMethod<T>;

  template <typename T>
  concept RandomExtensibleCollection = DynamicCollection<T>  //
                                       and HasEmplaceMethod<T>;

  template <typename T>
  concept SimpleCodeableAggregate =
      std::is_aggregate_v<std::remove_cvref_t<T>>  //
      and (not DynamicCollection<T>)               //
      and (not std::is_array_v<T>)                 //
      and (not detail::is_std_array<T>)            //
      and (detail::field_number_of<T> <= detail::MAX_FIELD_NUM);

  template <typename T, typename = void>
  struct HasDecomposeAndApply : std::false_type {};

  template <typename T>
  struct HasDecomposeAndApply<T,
                              std::void_t<decltype(decompose_and_apply(
                                  std::declval<T>(), [](auto &&...) {}))>>
      : std::true_type {};

  template <typename T>
  concept CustomDecomposable =
      not SimpleCodeableAggregate<T> and HasDecomposeAndApply<T>::value;

}  // namespace scale
