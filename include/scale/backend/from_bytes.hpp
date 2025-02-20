/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines the FromBytes backend for SCALE decoding.
 *
 * This file contains the implementation of the `FromBytes` class,
 * which serves as a backend for SCALE decoding, processing encoded
 * bytes from an input buffer and providing methods to extract data.
 */

#pragma once

#include <scale/decoder.hpp>

namespace scale::backend {

  /**
   * @class FromBytes
   * @brief Decoder backend that processes bytes from a span buffer.
   */
  class FromBytes final : public Decoder {
   public:
    /**
     * @brief Constructs a FromBytes decoder with an input byte span.
     * @param data The input data buffer to decode from.
     */
    explicit FromBytes(ConstSpanOfBytes data) : bytes_(data) {}

    /**
     * @brief Constructs a FromBytes decoder with an input byte span and
     * additional configurations.
     * @param data The input data buffer to decode from.
     * @param args Additional configuration parameters.
     */
    template <typename... Args>
    FromBytes(ConstSpanOfBytes data, const Args &...args)
        : Decoder(args...), bytes_(data) {}

    FromBytes(FromBytes &&) noexcept = delete;
    FromBytes(const FromBytes &) = delete;
    FromBytes &operator=(FromBytes &&) noexcept = delete;
    FromBytes &operator=(const FromBytes &) = delete;

    /**
     * @brief Checks if there are at least `amount` bytes available for reading.
     * @param amount The number of bytes to check.
     * @return True if enough bytes are available, false otherwise.
     */
    [[nodiscard]] bool has(size_t amount) const override {
      return bytes_.size() >= amount;
    }

    /**
     * @brief Takes and removes the next byte from the buffer.
     * @return The next byte.
     * @throws DecodeError::NOT_ENOUGH_DATA if there are no more bytes to read.
     */
    uint8_t take() override {
      if (bytes_.empty()) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }
      uint8_t byte = bytes_.front();
      bytes_ = bytes_.subspan(1);
      return byte;
    }

    /**
     * @brief Reads the specified number of bytes and updates the internal
     * buffer.
     * @param amount Number of bytes to read.
     * @return Span of the next required bytes.
     * @throws DecodeError::NOT_ENOUGH_DATA if not enough bytes are available.
     */
    std::span<const uint8_t> read(size_t amount) override {
      if (bytes_.size() < amount) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }
      auto result = bytes_.subspan(0, amount);
      bytes_ = bytes_.subspan(amount);
      return result;
    }

   private:
    /// Internal reference to input byte buffer.
    std::span<const uint8_t> bytes_;
  };

}  // namespace scale::backend