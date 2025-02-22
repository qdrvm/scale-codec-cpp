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

#include <cstdint>
#include <ranges>
#include <span>
#include <type_traits>

#include <scale/encoder.hpp>

namespace scale::backend {

  /**
   * @brief Concept to check if a container supports byte insertion at the end.
   *
   * This concept ensures that the container type supports:
   * - `push_back(uint8_t)` for adding a single byte at the end.
   * - `insert(iterator, iterator)` for inserting a range of bytes at the end.
   *
   * The container must:
   * - Be a template with `uint8_t` as the value type.
   * - Support `push_back(uint8_t)`.
   * - Support `insert(container.end(), begin, end)` for a range of iterators.
   *
   * Containers that typically satisfy this concept:
   * - `std::vector<uint8_t>`
   * - `std::deque<uint8_t>`
   * - `std::list<uint8_t>`
   *
   * This concept is used to ensure that `ToBytes` can efficiently add bytes
   * to the end of the container.
   *
   * @tparam T Template type of the container.
   */
  template <typename T>
  concept ByteReceiver =
      requires(T &container, uint8_t byte, std::span<const uint8_t> span) {
        { container.push_back(byte) } -> std::same_as<void>;
        {
          container.insert(container.end(), span.begin(), span.end())
        } -> std::same_as<typename T::iterator>;
      };

  /**
   * @class ToBytes
   * @brief Encoder backend that accumulates bytes into a container of uint8_t.
   *
   * This class is designed to efficiently add bytes to the end of a container,
   * supporting both single bytes and sequences of bytes. It provides an
   * abstraction for byte encoding that is compatible with the following
   * containers:
   * - `std::vector<uint8_t>`
   * - `std::deque<uint8_t>`
   * - `std::list<uint8_t>`
   *
   * @tparam Out The exact type of the container to accumulate bytes into.
   */
  template <typename Out = std::vector<uint8_t>>
    requires ByteReceiver<Out>
  class ToBytes final : public Encoder {
   public:
    /**
     * @brief Constructs a ToBytes encoder with a reference to the output
     * container.
     *
     * The reference is stored without copying, ensuring that all encoded bytes
     * are directly added to the provided container.
     *
     * @param container Reference to the container where bytes will be added.
     *
     * @note The container must outlive the ToBytes object to avoid dangling
     * references.
     */
    explicit ToBytes(Out &container) : out_(container) {}

    /**
     * @brief Constructs a ToBytes encoder with optional configuration.
     * @tparam Args Variadic template parameters for configuration.
     * @param args Configuration arguments.
     */
    template <typename... Args>
    ToBytes(Out &container, Args &&...args)
        : Encoder(std::forward<Args>(args)...), out_(container){};

    /// @brief Deleted default constructor to ensure container is passed.
    ToBytes() = delete;

    /// @brief Deleted move constructor.
    ToBytes(ToBytes &&) noexcept = delete;

    /// @brief Deleted copy constructor.
    ToBytes(const ToBytes &) = delete;

    /// @brief Deleted move assignment operator.
    ToBytes &operator=(ToBytes &&) noexcept = delete;

    /// @brief Deleted copy assignment operator.
    ToBytes &operator=(const ToBytes &) = delete;

    /**
     * @brief Checks if the data receiver is a continuous range.
     *
     * This method uses std::ranges::contiguous_range to determine if the
     * underlying data is stored contiguously in memory.
     *
     * @return true if the data receiver is contiguous, false otherwise.
     */
    [[nodiscard]] constexpr bool isContinuousReceiver() const override {
      return std::ranges::contiguous_range<decltype(out_)>;
    }

    /**
     * @brief Adds a single byte to the encoded buffer.
     *
     * This method appends a single byte to the end of the output container.
     * It utilizes `push_back(uint8_t)` to efficiently add the byte.
     *
     * @param byte The byte to be added.
     */
    void put(uint8_t byte) override {
      out_.push_back(byte);
    }

    /**
     * @brief Writes a sequence of bytes to the encoded buffer.
     *
     * This method appends a sequence of bytes from the given span to the end
     * of the output container. It uses `insert` with iterators from the span
     * for optimal insertion.
     *
     * @param bytes Span of bytes to write.
     *
     * @note If the container is contiguous (e.g., std::vector), the bytes
     *       are added in one contiguous block for efficiency.
     */
    void write(std::span<const uint8_t> bytes) override {
      out_.insert(out_.end(), bytes.begin(), bytes.end());
    }

    /**
     * @brief Retrieves the size of the encoded buffer.
     *
     * This method returns the number of bytes currently stored in the output
     * container.
     *
     * @return The number of bytes in the encoded buffer.
     */
    [[nodiscard]] size_t size() const override {
      return out_.size();
    }

    /**
     * @brief Provides a view of the stored bytes as a range.
     *
     * If the container is contiguous (e.g., std::vector or std::deque),
     * this method returns a `std::span<const uint8_t>` pointing to the stored
     * bytes. Otherwise, it returns a `std::ranges::subrange` for non-contiguous
     * containers (e.g., std::list).
     *
     * @return A view of the stored bytes.
     */
    [[nodiscard]] auto view() const {
      if constexpr (std::ranges::contiguous_range<Out>) {
        return std::span<const uint8_t>(out_.data(), out_.size());
      } else {
        return std::ranges::subrange(out_.begin(), out_.end());
      }
    }

   private:
    Out &out_;  ///< Reference to the container for encoded bytes.
  };

}  // namespace scale::backend