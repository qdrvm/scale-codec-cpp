/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Implements encoding and decoding functions for smart pointers using
 * SCALE encoding.
 *
 * This file provides support for encoding and decoding `std::shared_ptr`,
 * `std::unique_ptr`, and `std::reference_wrapper` using the SCALE encoding
 * scheme. The implementation ensures safe serialization of pointer-based
 * structures while handling null values and object lifetimes appropriately.
 *
 * ## Encoding Strategy:
 * - `std::shared_ptr<T>`: Encodes the underlying object if the pointer is
 * non-null, otherwise raises an error.
 * - `std::unique_ptr<T>`: Similar to `shared_ptr`, encodes the object or raises
 * an error.
 * - `std::reference_wrapper<T>`: Encodes the referenced object directly.
 *
 * ## Decoding Strategy:
 * - `std::shared_ptr<T>`: Constructs a new shared instance of `T` and decodes
 * its contents.
 * - `std::unique_ptr<T>`: Creates a unique instance of `T` and decodes its
 * contents.
 * - `std::reference_wrapper<T>`: Decodes the referenced object directly.
 */

#pragma once

#include <memory>

#include <scale/detail/tagged.hpp>
#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale {

  /**
   * @brief Encodes a shared pointer if it is non-null.
   * @tparam T The type of the shared pointer.
   * @param sptr The shared pointer to encode.
   * @param encoder SCALE encoder.
   * @throws EncodeError::DEREF_NULLPOINTER if the pointer is null.
   */
  template <typename T>
  void encode(const std::shared_ptr<T> &sptr, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(sptr)>
  {
    if (sptr) {
      return encode(*sptr, encoder);
    }
    raise(EncodeError::DEREF_NULLPOINTER);
  }

  /**
   * @brief Encodes a unique pointer if it is non-null.
   * @tparam T The type of the unique pointer.
   * @param uptr The unique pointer to encode.
   * @param encoder SCALE encoder.
   * @throws EncodeError::DEREF_NULLPOINTER if the pointer is null.
   */
  template <typename T>
  void encode(const std::unique_ptr<T> &uptr, ScaleEncoder auto &encoder)
    requires NoTagged<decltype(uptr)>
  {
    if (uptr) {
      return encode(*uptr, encoder);
    }
    raise(EncodeError::DEREF_NULLPOINTER);
  }

  /**
   * @brief Encodes a `std::reference_wrapper` using SCALE encoding.
   * @tparam T The type referenced.
   * @param reference The reference wrapper to encode.
   * @param encoder SCALE encoder.
   */
  template <typename T>
  void encode(const std::reference_wrapper<T> &reference,
              ScaleEncoder auto &encoder)
    requires NoTagged<decltype(reference)>
  {
    encode(reference.get(), encoder);
  }

  /**
   * @brief Decodes a `std::shared_ptr<T>` by constructing a new instance.
   * @tparam T The type to decode.
   * @param v The shared pointer where the decoded value will be stored.
   * @param decoder SCALE decoder.
   */
  template <typename T>
    requires std::is_default_constructible_v<std::remove_const_t<T>>
  void decode(std::shared_ptr<T> &v, ScaleDecoder auto &decoder) {
    v = std::make_shared<std::remove_const_t<T>>();
    decode(const_cast<std::remove_const_t<T> &>(*v), decoder);
  }

  /**
   * @brief Decodes a `std::unique_ptr<T>` by constructing a new instance.
   * @tparam T The type to decode.
   * @param v The unique pointer where the decoded value will be stored.
   * @param decoder SCALE decoder.
   */
  template <typename T>
    requires std::is_default_constructible_v<std::remove_const_t<T>>
  void decode(std::unique_ptr<T> &v, ScaleDecoder auto &decoder) {
    v = std::make_unique<std::remove_const_t<T>>();
    decode(const_cast<std::remove_const_t<T> &>(*v), decoder);
  }

  /**
   * @brief Decodes a `std::reference_wrapper<T>` using SCALE encoding.
   * @tparam T The type referenced.
   * @param reference The reference wrapper where the decoded value will be
   * stored.
   * @param decoder SCALE decoder.
   */
  template <typename T>
  void decode(std::reference_wrapper<T> &reference, ScaleDecoder auto &decoder)
    requires NoTagged<decltype(reference)>
  {
    decode(reference.get(), decoder);
  }

}  // namespace scale
