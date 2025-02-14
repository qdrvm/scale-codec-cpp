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
    FromBytes(ConstSpanOfBytes data) : bytes_(data) {};

    template <typename... Args>
    FromBytes(ConstSpanOfBytes data, const Args&... args)
        : Decoder(args...), bytes_(data){};

    FromBytes(FromBytes &&) noexcept = delete;
    FromBytes(const FromBytes &) = delete;
    FromBytes &operator=(FromBytes &&) noexcept = delete;
    FromBytes &operator=(FromBytes const &) = delete;

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
     */
    uint8_t take() override {
      if (bytes_.size() < 1) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }
      auto &&byte = bytes_.front();
      bytes_ = bytes_.last(bytes_.size() - 1);
      return byte;
    }

    /**
     * @brief Reads a sequence of bytes into the provided output span.
     * @param out The span to store the read bytes.
     */
    void read(std::span<uint8_t> out) override {
      if (bytes_.size() < out.size()) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }
      std::memcpy(out.data(), bytes_.data(), out.size());
      bytes_ = bytes_.last(bytes_.size() - out.size());
    }

   private:
    /// Internal reference to input byte buffer.
    std::span<const uint8_t> bytes_;
    size_t all_;
  };

}  // namespace scale::backend