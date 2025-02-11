/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines the ToBytes backend for SCALE encoding.
 *
 * This file contains the implementation of the `ToBytes` class,
 * which serves as a backend for SCALE encoding, storing encoded
 * bytes in an internal buffer and providing utility functions for
 * retrieving the encoded data.
 */

#pragma once

#include <scale/encoder_backend.hpp>

#include <deque>

namespace scale::backend {

  /**
   * @class ToBytes
   * @brief Encoder backend that accumulates bytes into a deque.
   */
  class ToBytes final : public EncoderBackend {
   public:
    ToBytes() = default;
    ToBytes(ToBytes &&) noexcept = delete;
    ToBytes(const ToBytes &) = delete;
    ToBytes &operator=(ToBytes &&) noexcept = delete;
    ToBytes &operator=(ToBytes const &) = delete;

    /**
     * @brief Adds a single byte to the encoded buffer.
     * @param byte The byte to be added.
     */
    void put(uint8_t byte) override {
      bytes_.push_back(byte);
    }

    /**
     * @brief Writes a sequence of bytes to the encoded buffer.
     * @param byte Span of bytes to write.
     */
    void write(std::span<const uint8_t> bytes) override {
      bytes_.insert(bytes_.end(), bytes.begin(), bytes.end());
    }

    /**
     * @brief Retrieves the size of the encoded buffer.
     * @return The number of bytes currently stored.
     */
    [[nodiscard]] size_t size() const override {
      return bytes_.size();
    }

    /**
     * @brief Returns a copy of the stored bytes as a vector.
     * @return A vector containing all stored bytes.
     */
    [[nodiscard]] std::vector<uint8_t> to_vector() const & {
      return {bytes_.begin(), bytes_.end()};
    }

    /**
     * @brief Moves the stored bytes into a vector.
     * @return A vector containing all stored bytes.
     */
    [[nodiscard]] std::vector<uint8_t> to_vector() && {
      return {std::make_move_iterator(bytes_.begin()),
              std::make_move_iterator(bytes_.end())};
    }

   private:
    std::deque<uint8_t> bytes_;  ///< Internal storage for encoded bytes.
  };

}  // namespace scale::backend
