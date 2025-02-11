/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/detail/collections_type_traits.hpp>
#include <scale/detail/type_traits.hpp>
#include <scale/detail/custom_decomposition.hpp>

namespace scale::detail::decomposable {

  namespace custom {

    template <typename T>
    concept is_custom_decomposable = requires(T &obj, const T &cobj) {
      {
        obj._custom_decompose_and_apply([](auto &&...) {})
      };
      {
        cobj._custom_decompose_and_apply([](auto &&...) {})
      };
    };

    template <typename T>
    concept CustomDecomposable = is_custom_decomposable<std::remove_cvref_t<T>>;

  }  // namespace custom

  namespace array {

    template <typename T>
    concept is_array =
        std::is_bounded_array_v<std::remove_cvref_t<T>>
        or (std::same_as<
            std::remove_cvref_t<T>,
            std::array<typename std::remove_cvref_t<T>::value_type,
                       std::tuple_size<std::remove_cvref_t<T>>::value>>);

    template <typename T>
    struct array_size_impl;

    template <typename T, std::size_t N>
    struct array_size_impl<std::array<T, N>> {
      static constexpr std::size_t value = N;
    };

    template <typename T, std::size_t N>
    struct array_size_impl<T[N]> {
      static constexpr std::size_t value = N;
    };

    template <typename T>
    inline constexpr std::size_t array_size =
        array_size_impl<std::remove_cvref_t<T>>::value;

    template <typename T, std::size_t N>
    concept ArrayWithMaxSize = is_array<T> and array_size<T> <= N;

    template <typename T>
    concept DecomposableArray = not custom::CustomDecomposable<T>
                                and ArrayWithMaxSize<T, MAX_FIELD_NUM>;

    template <typename F>
    decltype(auto) decompose_and_apply(DecomposableArray auto &&v, const F &f) {
      return decompose_and_apply<array_size<decltype(v)>>(
          std::forward<decltype(v)>(v), f);
    }

  }  // namespace array

  namespace structurally_bindable {

    template <typename, typename = void>
    struct is_structurally_bindable : std::false_type {};

    template <typename T>
    struct is_structurally_bindable<
        T,
        std::void_t<decltype(std::tuple_size<T>::value)>> : std::true_type {};

    template <typename, typename = void>
    struct structured_binding_size : std::integral_constant<std::size_t, 0> {};

    template <typename T>
    struct structured_binding_size<
        T,
        std::void_t<decltype(std::tuple_size<T>::value)>> : std::tuple_size<T> {
    };

    template <typename T>
    constexpr std::size_t structured_binding_size_v =
        structured_binding_size<std::remove_cvref_t<T>>::value;

    template <typename T>
    concept StructurallyBindable =
        is_structurally_bindable<std::remove_cvref_t<T>>::value;

    template <typename T>
    concept StructurallyBindableWithMaxSize =
        StructurallyBindable<T>
        and structured_binding_size_v<std::remove_cvref_t<T>> <= MAX_FIELD_NUM;

    template <typename T>
    concept DecomposableStructurallyBindable =
        (not custom::CustomDecomposable<T>)
        and (not array::DecomposableArray<T>)
        and StructurallyBindableWithMaxSize<T>;

    template <typename F>
    decltype(auto) decompose_and_apply(
        DecomposableStructurallyBindable auto &&v, const F &f) {
      return decompose_and_apply<structured_binding_size_v<decltype(v)>>(
          std::forward<decltype(v)>(v), f);
    }

  }  // namespace structurally_bindable

  namespace aggregate {

    using namespace common;

    template <typename T, int N = 0>
      requires std::is_aggregate_v<T>
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
        and std::same_as<std::remove_cvref_t<T>,
                         std::array<typename std::remove_cvref_t<T>::value_type,
                                    std::tuple_size_v<std::remove_cvref_t<T>>>>;

    template <typename T>
    concept Aggregate = std::is_aggregate_v<std::remove_cvref_t<T>>;

    template <typename T>
    concept AggregateWithMaxSize =
        Aggregate<T> and (field_number_of<T> <= MAX_FIELD_NUM);

    template <typename T>
    concept DecomposableAggregate =
        (not custom::CustomDecomposable<T>)
        and (not array::DecomposableArray<T>)
        and (not structurally_bindable::DecomposableStructurallyBindable<T>)
        and (not collections::Collection<T>) and AggregateWithMaxSize<T>;

    template <typename F>
    decltype(auto) decompose_and_apply(DecomposableAggregate auto &&v,
                                       const F &f) {
      return decompose_and_apply<field_number_of<decltype(v)>>(
          std::forward<decltype(v)>(v), f);
    }

  }  // namespace aggregate

  using aggregate::DecomposableAggregate;
  using array::DecomposableArray;
  using custom::CustomDecomposable;
  using structurally_bindable::DecomposableStructurallyBindable;

  struct NoDecompose {};

  template <typename T>
  concept IsNotDecomposable =
      std::derived_from<std::remove_cvref_t<T>, NoDecompose>;

  template <typename T>
  concept Decomposable = not IsNotDecomposable<T>
                         and (CustomDecomposable<T>    //
                              or DecomposableArray<T>  //
                              or DecomposableStructurallyBindable<T>
                              or DecomposableAggregate<std::remove_cvref_t<T>>);

}  // namespace scale::detail::decomposable