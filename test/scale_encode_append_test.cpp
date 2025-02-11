/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <qtils/test/outcome.hpp>

#include <scale/encode_append.hpp>
#include <scale/scale.hpp>

using scale::append_or_new_vec;
using scale::impl::with_buffer::encode;

using Values = std::vector<uint16_t>;

TEST(EncodeAppend, Expend_from_0_to_1024) {
  Values values;

  ASSERT_OUTCOME_SUCCESS(expandable, encode(values));

  uint16_t value = 0;
  for ([[maybe_unused]] auto i : std::views::iota(0, 1024)) {
    values.emplace_back(++value);

    ASSERT_OUTCOME_SUCCESS(expending, encode(value));
    ASSERT_OUTCOME_SUCCESS(append_or_new_vec(expandable, expending));

    ASSERT_OUTCOME_SUCCESS(direct_encoded, encode(values));

    ASSERT_EQ(expandable.size(), direct_encoded.size());
    EXPECT_EQ(expandable, direct_encoded);
  }
}
