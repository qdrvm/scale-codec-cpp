/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines the Decoder class for SCALE deserialization.
 *
 * This class provides a generic decoding mechanism using a configurable
 * backend. It supports custom configurations when enabled at compile-time.
 */

#pragma once

#include <span>

#include <scale/configurable.hpp>
#include <scale/detail/type_traits.hpp>
#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>

namespace scale {

  /**
   * @class Decoder
   * @brief A generic SCALE decoder using a backend for byte extraction.
   * @tparam DecoderBackendT The backend type that implements decoding logic.
   *
   * This class extends Configurable and provides an interface to decode data
   * using a backend. It supports custom configurations when enabled at
   * compile-time.
   */
  class Decoder : public Configurable {
   public:
    // struct Z {};
    //
    // static constexpr auto N =
    // detail::common::max_constructor_args<BackendType>;
    //
    // /**
    //  * @brief Constructs a Decoder with backend and configuration parameters.
    //  * @tparam I1 Index sequence for backend constructor arguments.
    //  * @tparam I2 Index sequence for configuration constructor arguments.
    //  * @tparam Args Variadic template arguments.
    //  */
    // template <std::size_t... I1, std::size_t... I2, typename... Args>
    // explicit Decoder(Z,
    //                  std::index_sequence<I1...>,
    //                  std::index_sequence<I2...>,
    //                  Args &&...args)
    //     : Configurable(std::get<I2>(std::forward_as_tuple(args...))...),
    //       backend_(std::get<I1>(std::forward_as_tuple(args...))...) {}
    //
    // /**
    //  * @brief Constructs a Decoder with a specified number of configuration
    //  * arguments.
    //  * @tparam N Number of configuration arguments.
    //  * @tparam Args Variadic template arguments.
    //  */
    // template <std::size_t N, typename... Args>
    // explicit Decoder(Args &&...args)
    //     : Decoder(Z{},
    //               std::make_index_sequence<sizeof...(Args) - N>{},
    //               std::make_index_sequence<N>{},
    //               std::forward<Args>(args)...) {}

#ifdef CUSTOM_CONFIG_ENABLED
    /**
     * @brief Constructs a decoder with custom configurations.
     * @param data Immutable span of bytes to decode.
     * @param configs Configuration parameters.
     */
    explicit Decoder(const MaybeConfig auto &...configs)
        : Configurable(configs...) {}
#else
    /**
     * @brief Constructs a decoder without custom configurations.
     * @tparam Args Variadic template arguments for backend initialization.
     */
    template <typename... Args>
    explicit Decoder(Args &&...args) : backend_(std::forward<Args>(args)...) {}
#endif

    /**
     * @brief Checks whether n more bytes are available.
     * @param n Number of bytes to check.
     * @return True if n more bytes are available, false otherwise.
     */
    [[nodiscard]] virtual bool has(size_t amount) const = 0;

    /**
     * @brief Takes one byte from the backend.
     * @return The byte read from the backend.
     */
    virtual uint8_t take() = 0;

    /**
     * @brief Reads a span of bytes into the provided output buffer.
     * @param out The span where the read bytes will be stored.
     */
    virtual void read(std::span<uint8_t> out) = 0;
  };

  /**
   * @brief Decodes a value using the backend.
   * @tparam T The type of the value to decode.
   * @param value The value to decode.
   * @return Reference to the decoder for chaining operations.
   */
  template <typename T>
  Decoder &operator>>(Decoder &decoder, T &&value) {
    decode(std::forward<T>(value), decoder);
    return decoder;
  }

}  // namespace scale
