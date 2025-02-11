/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines concepts for handling various types of collections
 *        in SCALE serialization.
 *
 * This file provides utility concepts for identifying and classifying
 * different kinds of collections, including static, dynamic, resizable,
 * and extensible collections.
 */

#pragma once

#include <span>

#include <scale/detail/compact_integer.hpp>
#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale {

  namespace detail::collections {

    /**
     * @brief Concept for detecting std::span-like types.
     */
    template <typename T>
    concept SomeSpan =
        std::derived_from<T, std::span<typename T::element_type, T::extent>>;

    /**
     * @brief Concept for checking if a type has an insert method.
     */
    template <typename T>
    concept HasSomeInsertMethod = requires(T v) {
      v.insert(v.end(), *v.begin());
    } or requires(T v) { v.insert_after(v.end(), *v.begin()); };

    /**
     * @brief Concept for checking if a type has a resize method.
     */
    template <typename T>
    concept HasResizeMethod =
        requires(std::remove_cvref_t<T> v) { v.resize(v.size()); };

    /**
     * @brief Concept for checking if a type has a reserve method.
     */
    template <typename T>
    concept HasReserveMethod =
        requires(std::remove_cvref_t<T> v) { v.reserve(v.size()); };

    /**
     * @brief Concept for checking if a type has an emplace method.
     */
    template <typename T>
    concept HasEmplaceMethod =
        requires(std::remove_cvref_t<T> v) { v.emplace(*v.begin()); };

    /**
     * @brief Concept for checking if a type has an emplace_back method.
     */
    template <typename T>
    concept HasEmplaceBackMethod =
        requires(std::remove_cvref_t<T> v) { v.emplace_back(*v.begin()); };

    /**
     * @brief Concept for implicitly defining a static collection.
     */
    template <typename T>
    concept ImplicitlyDefinedAsStatic =
        not(SomeSpan<std::remove_cvref_t<T>>)
        and not(HasSomeInsertMethod<std::remove_cvref_t<T>>);

    /**
     * @brief Concept for implicitly defining a dynamic collection.
     */
    template <typename T>
    concept ImplicitlyDefinedAsDynamic =
        not(SomeSpan<std::remove_cvref_t<T>>)
        and HasSomeInsertMethod<std::remove_cvref_t<T>>;

    /**
     * @brief Concept for detecting static spans.
     */
    template <typename T>
    concept StaticSpan =
        SomeSpan<std::remove_cvref_t<T>> and (T::extent != std::dynamic_extent);

    /**
     * @brief Concept for detecting dynamic spans.
     */
    template <typename T>
    concept DynamicSpan =
        SomeSpan<std::remove_cvref_t<T>> and (T::extent == std::dynamic_extent);

    /**
     * @brief Concept for identifying static collections.
     */
    template <typename T>
    concept StaticCollection =
        std::ranges::range<std::remove_cvref_t<T>>
        and (ImplicitlyDefinedAsStatic<std::remove_cvref_t<T>>
             or StaticSpan<std::remove_cvref_t<T>>);

    /**
     * @brief Concept for identifying dynamic collections.
     */
    template <typename T>
    concept DynamicCollection =
        std::ranges::sized_range<std::remove_cvref_t<T>>
        and (ImplicitlyDefinedAsDynamic<std::remove_cvref_t<T>>
             or DynamicSpan<std::remove_cvref_t<T>>);

    /**
     * @brief Concept for any type of collection.
     */
    template <typename T>
    concept Collection = StaticCollection<T> or DynamicCollection<T>;

    /**
     * @brief Concept for resizable collections.
     */
    template <typename T>
    concept ResizeableCollection = DynamicCollection<std::remove_cvref_t<T>>
                                   and HasResizeMethod<std::remove_cvref_t<T>>;

    /**
     * @brief Concept for collections that support push-back operations.
     */
    template <typename T>
    concept ExtensibleBackCollection =
        DynamicCollection<std::remove_cvref_t<T>>
        and not(HasResizeMethod<std::remove_cvref_t<T>>)
        and HasEmplaceBackMethod<std::remove_cvref_t<T>>;

    /**
     * @brief Concept for collections that support random insertion operations.
     */
    template <typename T>
    concept RandomExtensibleCollection =
        DynamicCollection<std::remove_cvref_t<T>>
        and HasEmplaceMethod<std::remove_cvref_t<T>>;

  }  // namespace detail::collections

}  // namespace scale
