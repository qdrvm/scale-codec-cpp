/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Provides utilities for analyzing constructor properties of types.
 *
 * This file defines utilities for determining the number of default arguments
 * that can be used to construct a given type. It helps in template
 * metaprogramming to infer constructor capabilities at compile-time.
 */

#pragma once

namespace scale::detail::common {

  /**
   * @struct ArgHelper
   * @brief Utility struct used for constructing objects with default arguments.
   */
  struct ArgHelper {
    /**
     * @brief Implicit conversion operator to any type.
     * @tparam T The target type for conversion.
     * @return A default-constructed instance of T.
     */
    template <typename T>
    operator T() const {  // NOLINT(*-explicit-constructor)
      return T{};
    }
  };

  /**
   * @brief Checks if a type is constructible with N default arguments.
   * @tparam T The type to check.
   * @tparam Indices Index sequence representing argument positions.
   * @return True if T is constructible with N default arguments, false
   * otherwise.
   */
  template <typename T, std::size_t... Indices>
  constexpr bool is_constructible_with_n_def_args_impl(
      std::index_sequence<Indices...>) {
    return requires { T{(void(Indices), ArgHelper{})...}; };
  }

  /**
   * @brief Determines if a type T is constructible with exactly N default
   * arguments.
   * @tparam T The type to check.
   * @tparam N The number of default arguments.
   */
  template <typename T, size_t N>
  constexpr bool is_constructible_with_n_def_args_v =
      is_constructible_with_n_def_args_impl<T>(std::make_index_sequence<N>{});

  /**
   * @brief Recursively determines the maximum number of default constructor
   * arguments.
   * @tparam T The type to check.
   * @tparam N The current number of default arguments being tested.
   * @return The maximum number of default arguments supported by T.
   */
  template <typename T, std::size_t N = 0>
  constexpr std::size_t max_constructor_args_impl() {
    if constexpr (is_constructible_with_n_def_args_v<T, N + 1>) {
      return max_constructor_args_impl<T, N + 1>();
    } else {
      return N;
    }
  }

  /**
   * @brief Computes the maximum number of default constructor arguments for a
   * type.
   * @tparam T The type to check.
   */
  template <typename T>
  constexpr std::size_t max_constructor_args = max_constructor_args_impl<T>();

}  // namespace scale::detail::common