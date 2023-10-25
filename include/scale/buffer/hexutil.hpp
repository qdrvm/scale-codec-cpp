/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>
#include <vector>

#include <boost/algorithm/hex.hpp>

#include <scale/outcome/outcome.hpp>
#include <scale/types.hpp>

namespace scale {

  /**
   * @brief error codes for exceptions that may occur during unhexing
   */
  enum class UnhexError {
    NOT_ENOUGH_INPUT = 1,
    NON_HEX_INPUT,
    VALUE_OUT_OF_RANGE,
    MISSING_0X_PREFIX,
    UNKNOWN
  };

  /**
   * @brief Converts bytes to hex representation
   * @param array bytes
   * @param len length of bytes
   * @return hexstring
   */
  std::string hex_lower(ConstSpanOfBytes bytes) noexcept;

  /**
   * @brief Converts hex representation to bytes
   * @param array individual chars
   * @param len length of chars
   * @return result containing array of bytes if input string is hex encoded and
   * has even length
   *
   * @note reads both uppercase and lowercase hexstrings
   *
   * @see
   * https://www.boost.org/doc/libs/1_51_0/libs/algorithm/doc/html/the_boost_algorithm_library/Misc/hex.html
   */
  outcome::result<std::vector<uint8_t>> unhex(std::string_view hex);

  /**
   * @brief Unhex hex-string with 0x in the begining
   * @param hex hex string with 0x in the beginning
   * @return unhexed buffer
   */
  outcome::result<std::vector<uint8_t>> unhexWith0x(std::string_view hex);
}  // namespace scale

OUTCOME_HPP_DECLARE_ERROR_2(scale, UnhexError)
