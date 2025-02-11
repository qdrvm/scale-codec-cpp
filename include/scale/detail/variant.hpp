/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Provides utilities for handling std::variant and boost::variant
 *        in SCALE encoding/decoding.
 *
 * This file defines utilities to work with `std::variant` and `boost::variant`,
 * allowing efficient encoding and decoding using SCALE serialization.
 */

#pragma once

#include <variant>

#ifdef __has_include
#if __has_include(<boost/variant.hpp>)
#include <boost/variant.hpp>
#define HAS_BOOST_VARIANT
#endif
#endif

#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale {

  namespace detail::variant {
    /**
     * @brief Trait to check if a type is std::variant.
     */
    template <typename>
    struct is_std_variant_impl : std::false_type {};

    template <typename... Ts>
    struct is_std_variant_impl<std::variant<Ts...>> : std::true_type {};

    template <typename T>
    constexpr bool is_std_variant =
        is_std_variant_impl<std::remove_cvref_t<T>>::value;

    /**
     * @brief Concept for std::variant.
     */
    template <typename T>
    concept StdVariant = is_std_variant<T>;

    /**
     * @brief Trait to check if a type is boost::variant.
     */
    template <typename>
    struct is_boost_variant_impl : std::false_type {};

#ifdef HAS_BOOST_VARIANT
    template <typename... Ts>
    struct is_boost_variant_impl<boost::variant<Ts...>> : std::true_type {};
#endif

    template <typename T>
    constexpr bool is_boost_variant =
        is_boost_variant_impl<std::remove_cvref_t<T>>::value;

    /**
     * @brief Concept for boost::variant.
     */
    template <typename T>
    concept BoostVariant = is_boost_variant<T>;

    /**
     * @brief Concept for any variant type (std::variant or boost::variant).
     */
    template <typename T>
    concept Variant = StdVariant<T> or BoostVariant<T>;

    /**
     * @brief Converts a variant type to a tuple type.
     */
    template <typename, typename = void>
    struct to_tuple_impl {};

    template <template <typename...> class Template, typename... Args>
    struct to_tuple_impl<Template<Args...>, std::void_t<>> {
      using type = std::tuple<Args...>;
    };

    template <typename T>
    using to_tuple = typename to_tuple_impl<std::remove_cvref_t<T>>::type;

    /**
     * @brief Retrieves the type by index from a tuple.
     */
    template <std::size_t Index, typename Tuple>
      requires(Index < std::tuple_size_v<Tuple>)
    using type_by_index_impl = std::tuple_element_t<Index, Tuple>;

    template <std::size_t Index, typename T>
    using type_by_index = type_by_index_impl<Index, to_tuple<T>>;

    template <typename T>
      requires Variant<T>
    using VariantTypes = to_tuple<T>;

    /**
     * @brief Gets the size of a variant type.
     */
    template <typename T>
      requires Variant<T>
    constexpr size_t VariantSize = std::tuple_size_v<to_tuple<T>>;

    /**
     * @brief Retrieves the type of a variant by index.
     */
    template <std::size_t Index, typename T>
      requires Variant<T>
    using VariantType = type_by_index<Index, T>;

    /**
     * @brief Retrieves the value from a variant by type.
     */
    template <typename T>
    decltype(auto) get_variant(Variant auto &&var) {
      return get<T>(std::forward<decltype(var)>(var));
    }

    /**
     * @brief Retrieves the value from a variant by index.
     */
    template <size_t I>
    decltype(auto) get_variant(Variant auto &&var) {
      using T = VariantType<I, decltype(var)>;
      return get<T>(std::forward<decltype(var)>(var));
    }

    /**
     * @brief Retrieves the index of a std::variant.
     */
    auto variant_index(StdVariant auto &&var) {
      return var.index();
    }

    /**
     * @brief Retrieves the index of a boost::variant.
     */
    auto variant_index(BoostVariant auto &&var) {
      return var.which();
    }
  }  // namespace detail::variant

  using detail::variant::get_variant;
  using detail::variant::Variant;
  using detail::variant::variant_index;
  using detail::variant::VariantSize;
  using detail::variant::VariantType;
  using detail::variant::VariantTypes;

  /**
   * @brief Encodes a variant using SCALE encoding.
   * @tparam I The index of the variant type.
   * @param variant The variant to encode.
   * @param encoder The encoder to use.
   */
  template <uint8_t I>
  void encode(const Variant auto &variant, ScaleEncoder auto &encoder) {
    using T = decltype(variant);
    if (variant_index(variant) == I) {
      encode(I, encoder);
      encode(get_variant<I>(variant), encoder);
      return;
    }
    if constexpr (VariantSize<T> > I + 1) {
      encode<I + 1>(variant, encoder);
    }
  }

  /**
   * @brief Encodes a variant using SCALE encoding.
   * @param variant The variant to encode.
   * @param encoder The encoder to use.
   */
  void encode(const Variant auto &variant, ScaleEncoder auto &encoder) {
    encode<0>(variant, encoder);
  }

  /**
   * @brief Decodes a variant using SCALE decoding.
   * @tparam I The index of the variant type.
   * @param variant The variant to decode.
   * @param decoder The decoder to use.
   * @param index The index of the type to decode.
   */
  template <size_t I>
  void decode(Variant auto &variant,
              ScaleDecoder auto &decoder,
              const size_t index)
    requires(I < std::tuple_size_v<VariantTypes<decltype(variant)>>)
  {
    using T = decltype(variant);
    if (I == index) {
      static_assert(std::is_default_constructible_v<VariantType<I, T>>,
                    "All types of variant must be default constructible");
      VariantType<I, T> value;
      decode(value, decoder);
      variant = std::move(value);
      return;
    }
    if constexpr (VariantSize<T> > I + 1) {
      decode<I + 1>(variant, decoder, index);
    }
  }

  /**
   * @brief Decodes a variant using SCALE decoding.
   * @param variant The variant to decode.
   * @param decoder The decoder to use.
   */
  void decode(Variant auto &variant, ScaleDecoder auto &decoder) {
    using T = decltype(variant);
    uint8_t index = decoder.take();
    if (index < VariantSize<T>) {
      return decode<0>(variant, decoder, index);
    }
    raise(DecodeError::WRONG_TYPE_INDEX);
  }
}  // namespace scale
