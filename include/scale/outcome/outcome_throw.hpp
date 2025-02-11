/**
* Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Provides error handling utilities for SCALE serialization.
 *
 * This file defines a utility function for raising exceptions using Boost.
 */

#pragma once

#include <boost/throw_exception.hpp>
#include <system_error>

namespace scale {
  /**
   * @brief Raises a system error exception using Boost.
   * @param ec The error code to throw as an exception.
   * @throws std::system_error Always throws an exception with the given error code.
   */
  [[noreturn]] inline void raise(const std::error_code &ec) {
    boost::throw_exception(std::system_error(ec));
  }
}  // namespace scale