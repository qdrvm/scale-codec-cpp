/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * @brief Defines a custom decomposition mechanism and function application to
 * an object, allowing easy customization of encoding (e.g., using only specific
 * fields or changing their order).
 *
 * @param Self The name of the class where this macro is used.
 * @param ... The list of class members in the desired order that will
 * participate in decomposition.
 *
 * @attention Macro can be used only in public section!
 *
 * Example usage:
 * @code
 * struct Point {
 *   int x, y;
 *
 *   SCALE_CUSTOM_DECOMPOSITION(Point, x, y);
 * };
 * @endcode
 */
#define SCALE_CUSTOM_DECOMPOSITION(Self, ...)                                  \
  decltype(auto) _custom_decompose_and_apply(auto &&f) {                       \
    return std::forward<decltype(f)>(f)(__VA_ARGS__);                          \
  }                                                                            \
  decltype(auto) _custom_decompose_and_apply(auto &&f) const {                 \
    return std::forward<decltype(f)>(f)(__VA_ARGS__);                          \
  }                                                                            \
  template <typename F, typename V>                                            \
    requires std::derived_from<std::remove_cvref_t<V>, Self>                   \
  friend decltype(auto) decompose_and_apply(V &&v, F &&f) {                    \
    return std::forward<V>(v)._custom_decompose_and_apply(std::forward<F>(f)); \
  }
