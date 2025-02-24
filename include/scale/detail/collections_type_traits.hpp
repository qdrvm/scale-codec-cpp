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

#include <array>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <span>
#include <unordered_map>
#include <unordered_set>

#include <scale/detail/compact_integer.hpp>
#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale {

  namespace detail::collections {

    /**
     * @brief Concept to check if a type is an immutable collection.
     *
     * A type is considered an immutable collection if it has a nested type
     * `value_type` and that type is `const`.
     *
     * @tparam T The type to check.
     */
    template <typename T>
    concept ImmutableCollection = requires {
      typename T::value_type;
    } and std::is_const_v<std::remove_reference_t<typename T::value_type>>;

    // Don't change by default
    template <typename T>
    struct mutable_collection_impl {
      using type = T;
    };

    // For C-style array
    template <typename T, std::size_t N>
    struct mutable_collection_impl<T[N]> {
      using type = std::array<std::remove_const_t<T>, N>;
    };

    // For array
    template <typename T, std::size_t N>
    struct mutable_collection_impl<std::array<const T, N>> {
      using type = std::array<T, N>;
    };

    // For std::vector, std::deque, std::list, etc.
    template <template <typename, typename...> class Container,
              typename T,
              typename Alloc,
              typename... Args>
    struct mutable_collection_impl<Container<const T, Alloc, Args...>> {
      using MutableAlloc =
          typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
      using type = Container<T, MutableAlloc, Args...>;
    };

    // For std::set<const T, Compare, Alloc>
    template <typename T, typename Compare, typename Alloc>
    struct mutable_collection_impl<std::set<const T, Compare, Alloc>> {
      using MutableAlloc =
          typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
      using type = std::set<T, Compare, MutableAlloc>;
    };

    // For std::map<const Key, Value, Compare, Alloc>
    template <typename Key, typename Value, typename Compare, typename Alloc>
    struct mutable_collection_impl<std::map<const Key, Value, Compare, Alloc>> {
      using MutableAlloc = typename std::allocator_traits<
          Alloc>::template rebind_alloc<std::pair<const Key, Value>>;
      using type = std::map<Key, Value, Compare, MutableAlloc>;
    };

    // For std::unordered_set<const T, Hash, KeyEqual, Alloc>
    template <typename T, typename Hash, typename KeyEqual, typename Alloc>
    struct mutable_collection_impl<
        std::unordered_set<const T, Hash, KeyEqual, Alloc>> {
      using MutableAlloc =
          typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
      using type = std::unordered_set<T, Hash, KeyEqual, MutableAlloc>;
    };

    // For std::unordered_map<const Key, Value, Hash, KeyEqual, Alloc>
    template <typename Key,
              typename Value,
              typename Hash,
              typename KeyEqual,
              typename Alloc>
    struct mutable_collection_impl<
        std::unordered_map<const Key, Value, Hash, KeyEqual, Alloc>> {
      using MutableAlloc = typename std::allocator_traits<
          Alloc>::template rebind_alloc<std::pair<const Key, Value>>;
      using type = std::unordered_map<Key, Value, Hash, KeyEqual, MutableAlloc>;
    };

    /**
     * @brief Transform the mutable version of an immutable collection.
     * @tparam T The immutable collection type.
     */
    template <typename T>
    using mutable_collection_t = typename mutable_collection_impl<T>::type;

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
