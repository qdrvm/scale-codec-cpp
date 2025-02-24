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

/**
 * @brief A helper macro to cast the current object to its base type while
 * preserving its qualifiers.
 *
 * @param Base The base class type.
 *
 * Example usage:
 * @code
 * struct Base1 {
 *   ...
 * };
 * struct Base2 {
 *   Member2 member;
 * };
 * struct Derived : Base1, Base2 {
 *   Member our_member;
 *
 *   SCALE_CUSTOM_DECOMPOSITION(
 *     Derived,
 *     SCALE_FROM_BASE(Base1),        // coding whole Base1
 *     SCALE_FROM_BASE(Base2).member, // coding single member of Base2
 *     our_member);
 * };
 * @endcode
 */
#define SCALE_FROM_BASE(Base) \
  ((qtils::detail::tagged::copy_qualifiers_t<decltype(*this), Base>)(*this))
