/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <span>

#include <scale/detail/compact_integer.hpp>
#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale {

  namespace detail::collections {

    template <typename T>
    concept SomeSpan =
        std::derived_from<T, std::span<typename T::element_type, T::extent>>;

    template <typename T>
    concept HasSomeInsertMethod = requires(T v) {
      v.insert(v.end(), *v.begin());
    } or requires(T v) { v.insert_after(v.end(), *v.begin()); };

    template <typename T>
    concept HasResizeMethod =
        requires(std::remove_cvref_t<T> v) { v.resize(v.size()); };

    template <typename T>
    concept HasReserveMethod =
        requires(std::remove_cvref_t<T> v) { v.reserve(v.size()); };

    template <typename T>
    concept HasEmplaceMethod =
        requires(std::remove_cvref_t<T> v) { v.emplace(*v.begin()); };

    template <typename T>
    concept HasEmplaceBackMethod =
        requires(std::remove_cvref_t<T> v) { v.emplace_back(*v.begin()); };

    template <typename T>
    concept ImplicitlyDefinedAsStatic =
        not(SomeSpan<std::remove_cvref_t<T>>) and  //
        not(HasSomeInsertMethod<std::remove_cvref_t<T>>);

    template <typename T>
    concept ImplicitlyDefinedAsDynamic =
        not(SomeSpan<std::remove_cvref_t<T>>) and  //
        HasSomeInsertMethod<std::remove_cvref_t<T>>;

    template <typename T>
    concept StaticSpan = SomeSpan<std::remove_cvref_t<T>>  //
                         and (T::extent != std::dynamic_extent);

    template <typename T>
    concept DynamicSpan = SomeSpan<std::remove_cvref_t<T>>  //
                          and (T::extent == std::dynamic_extent);

    template <typename T>
    concept StaticCollection =
        std::ranges::range<std::remove_cvref_t<T>>
        and (ImplicitlyDefinedAsStatic<std::remove_cvref_t<T>>
             or StaticSpan<std::remove_cvref_t<T>>);

    template <typename T>
    concept DynamicCollection =
        std::ranges::sized_range<std::remove_cvref_t<T>>
        and (ImplicitlyDefinedAsDynamic<std::remove_cvref_t<T>>
             or DynamicSpan<std::remove_cvref_t<T>>);

    template <typename T>
    concept Collection = StaticCollection<T> or DynamicCollection<T>;

    template <typename T>
    concept ResizeableCollection = DynamicCollection<std::remove_cvref_t<T>>
                                   and HasResizeMethod<std::remove_cvref_t<T>>;

    template <typename T>
    concept ExtensibleBackCollection =
        DynamicCollection<std::remove_cvref_t<T>>
        and not(HasResizeMethod<std::remove_cvref_t<T>>)
        and HasEmplaceBackMethod<std::remove_cvref_t<T>>;

    template <typename T>
    concept RandomExtensibleCollection =
        DynamicCollection<std::remove_cvref_t<T>>
        and HasEmplaceMethod<std::remove_cvref_t<T>>;

  }  // namespace detail::collections

}  // namespace scale