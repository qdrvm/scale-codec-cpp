/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines utilities for decomposing and applying operations
 *        on decomposable types in SCALE serialization.
 *
 * This file provides functions for handling decomposition and
 * encoding/decoding of various decomposable types such as arrays,
 * structurally bindable types, and aggregates.
 */

#pragma once

#include <scale/detail/decomposable_type_traits.hpp>
#include <scale/detail/decompose_and_apply.hpp>
#include <scale/detail/tagged.hpp>

namespace scale {

  namespace detail {

    using namespace decomposable::array;

    /**
     * @brief Decomposes and applies a function to a decomposable array.
     * @tparam F The function to apply.
     * @param v The decomposable array.
     * @param f The function to apply to decomposed elements.
     * @return Result of applying the function.
     */
    template <typename F>
    decltype(auto) decompose_and_apply(DecomposableArray auto &&v, const F &f) {
      return decompose_and_apply<array_size<decltype(v)>>(
          std::forward<decltype(v)>(v), f);
    }

    using namespace decomposable::structurally_bindable;

    /**
     * @brief Decomposes and applies a function to a structurally bindable type.
     * @tparam F The function to apply.
     * @param v The decomposable structurally bindable type.
     * @param f The function to apply to decomposed elements.
     * @return Result of applying the function.
     */
    template <typename F>
    decltype(auto) decompose_and_apply(
        DecomposableStructurallyBindable auto &&v, const F &f) {
      return decompose_and_apply<structured_binding_size_v<decltype(v)>>(
          std::forward<decltype(v)>(v), f);
    }

    using namespace decomposable::aggregate;

    /**
     * @brief Decomposes and applies a function to an aggregate type.
     * @tparam F The function to apply.
     * @param v The decomposable aggregate type.
     * @param f The function to apply to decomposed elements.
     * @return Result of applying the function.
     */
    template <typename F>
    decltype(auto) decompose_and_apply(DecomposableAggregate auto &&v,
                                       const F &f) {
      return decompose_and_apply<field_number_of<decltype(v)>>(
          std::forward<decltype(v)>(v), f);
    }

  }  // namespace detail

  using detail::decompose_and_apply;

  /**
   * @brief Encodes a decomposable object using SCALE encoding.
   * @tparam decomposable The decomposable type to encode.
   * @param encoder The encoder instance to write to.
   */
  void encode(Decomposable auto &&decomposable, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(decomposable)>
  {
    decompose_and_apply(std::forward<decltype(decomposable)>(decomposable),
                        [&](auto &&...args) {
                          (encode(std::forward<decltype(args)>(args), encoder),
                           ...);
                        });
  }

  /**
   * @brief Decodes a decomposable object using SCALE decoding.
   * @tparam decomposable The decomposable type to decode.
   * @param decoder The decoder instance to read from.
   */
  void decode(Decomposable auto &decomposable, ScaleDecoder auto &decoder)
    requires NoTagged<decltype(decomposable)>
  {
    return decompose_and_apply(decomposable, [&](auto &&...args) {
        (decode(const_cast<std::remove_cv_t<std::remove_reference_t<decltype(args)>>&>(args), decoder), ...);
    });
  }

}  // namespace scale
