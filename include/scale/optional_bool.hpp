/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief A lightweight implementation of std::optional<bool> using a single
 * byte.
 *
 * The first bit (bit 0) indicates whether a value is present.
 * The second bit (bit 1) stores the boolean value.
 */

#pragma once

#include <cstdint>
#include <stdexcept>

namespace scale {

  /**
   * @class OptionalBool
   * @brief A compact implementation of std::optional<bool> using a single byte.
   *
   * Internal representation:
   *  - 0: no value (std::nullopt)
   *  - 1: true
   *  - 2: false
   */
  class OptionalBool {
   public:
    /**
     * @brief Default constructor. Initializes an empty OptionalBool
     * (std::nullopt).
     */
    constexpr OptionalBool() noexcept : data_(0) {}

    /**
     * @brief Constructs an empty OptionalBool using std::nullopt.
     * @param none std::nullopt constant.
     */
    constexpr OptionalBool(std::nullopt_t) noexcept : data_(0) {}

    /**
     * @brief Constructs an OptionalBool with a given boolean value.
     * @param value The boolean value to store.
     */
    constexpr OptionalBool(bool value) noexcept {
      set(value);
    }

    /**
     * @brief Checks whether the OptionalBool contains a value.
     * @return True if a value is present, false otherwise.
     */
    constexpr bool has_value() const noexcept {
      return data_ != 0;
    }

    /**
     * @brief Explicit boolean conversion to check if a value is present.
     */
    constexpr explicit operator bool() const noexcept {
      return has_value();
    }

    /**
     * @brief Retrieves the stored boolean value.
     * @return The stored boolean value.
     * @throws std::logic_error if no value is present.
     */
    bool value() const {
      if (!has_value()) throw std::logic_error("OptionalBool has no value");
      return data_ == 1;
    }

    /**
     * @brief Dereference operator for accessing the stored value.
     * @return The stored boolean value.
     */
    bool operator*() const {
      return value();
    }

    /**
     * @brief Retrieves the stored value or a default value if none is present.
     * @param default_value The default value to return if no value is present.
     * @return The stored value if present, otherwise default_value.
     */
    constexpr bool value_or(bool default_value) const noexcept {
      return has_value() ? (data_ == 1) : default_value;
    }

    /**
     * @brief Resets the OptionalBool to an empty state.
     */
    constexpr void reset() noexcept {
      data_ = 0;
    }

    /**
     * @brief Assigns a boolean value to the OptionalBool.
     * @param value The boolean value to assign.
     * @return Reference to the modified OptionalBool.
     */
    OptionalBool &operator=(bool value) noexcept {
      set(value);
      return *this;
    }

    /**
     * @brief Assigns std::nullopt to the OptionalBool, resetting its state.
     * @param none std::nullopt constant.
     * @return Reference to the modified OptionalBool.
     */
    OptionalBool &operator=(std::nullopt_t) noexcept {
      reset();
      return *this;
    }

    /**
     * @brief Equality comparison operator.
     * @param lhs First OptionalBool.
     * @param rhs Second OptionalBool.
     * @return True if both are equal, otherwise false.
     */
    friend constexpr bool operator==(const OptionalBool &lhs,
                                     const OptionalBool &rhs) noexcept {
      return lhs.data_ == rhs.data_;
    }

    /**
     * @brief Inequality comparison operator.
     * @param lhs First OptionalBool.
     * @param rhs Second OptionalBool.
     * @return True if both are not equal, otherwise false.
     */
    friend constexpr bool operator!=(const OptionalBool &lhs,
                                     const OptionalBool &rhs) noexcept {
      return !(lhs == rhs);
    }

    /**
     * @brief Encodes the optional bool into a SCALE encoder.
     * @param opt_bool The optional bool to encode.
     * @param encoder The encoder instance to write into.
     */
    friend void encode(const OptionalBool &opt_bool, Encoder &encoder) {
      encoder.put(opt_bool.data_);
    }

    /**
     * @brief Decodes a BitVec from a SCALE decoder.
     * @param opt_bool The optional bool instance to decode into.
     * @param decoder The decoder instance to read from.
     * @throws DecodeError::NOT_ENOUGH_DATA If there is insufficient data.
     * @throws DecodeError::UNEXPECTED_VALUE If there is wrong data.
     */
    friend void decode(OptionalBool &opt_bool, Decoder &decoder) {
      auto byte = decoder.take();
      switch (byte) {
        case 0:
          opt_bool.reset();
          return;
        case 1:
          opt_bool = true;
          return;
        case 2:
          opt_bool = false;
          return;
        default:
          raise(DecodeError::UNEXPECTED_VALUE);
      }
    }

   private:
    uint8_t data_;  ///< Internal data representing the state.

    /**
     * @brief Sets the internal data to represent the given boolean value.
     * @param value The boolean value to set.
     */
    constexpr void set(bool value) noexcept {
      data_ = value ? 1 : 2;
    }
  };

}  // namespace scale