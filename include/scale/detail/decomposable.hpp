/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <scale/detail/decompose_and_apply.hpp>
#include <scale/detail/decomposable_type_traits.hpp>

namespace scale {

  namespace detail {

    using namespace decomposable::array;

    template <typename F>
    decltype(auto) decompose_and_apply(DecomposableArray auto &&v, const F &f) {
      return decompose_and_apply<array_size<decltype(v)>>(
          std::forward<decltype(v)>(v), f);
    }

    using namespace decomposable::structurally_bindable;

    template <typename F>
    decltype(auto) decompose_and_apply(
        DecomposableStructurallyBindable auto &&v, const F &f) {
      return decompose_and_apply<structured_binding_size_v<decltype(v)>>(
          std::forward<decltype(v)>(v), f);
    }

    using namespace decomposable::aggregate;

    template <typename F>
    decltype(auto) decompose_and_apply(DecomposableAggregate auto &&v,
                                       const F &f) {
      return decompose_and_apply<field_number_of<decltype(v)>>(
          std::forward<decltype(v)>(v), f);
    }

  }  // namespace detail

  using detail::decompose_and_apply;

  void encode(Decomposable auto &&decomposable, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(decomposable)>
  {
    decompose_and_apply(std::forward<decltype(decomposable)>(decomposable),
                        [&](auto &...args) { (encode(args, encoder), ...); });
  }

  void decode(Decomposable auto &decomposable, ScaleDecoder auto &decoder)
    requires NoTagged<decltype(decomposable)>
  {
    return decompose_and_apply(decomposable, [&](auto &...args) {
      (decode(const_cast<std::remove_cvref_t<decltype(args)> &>(args), decoder),
       ...);
    });
  }

}  // namespace scale