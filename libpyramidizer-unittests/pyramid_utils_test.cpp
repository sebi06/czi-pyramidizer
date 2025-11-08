// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include "inc/pyramid_utilities.h"

using namespace std;
using namespace libpyramidizer;

struct EnlargeAndLatchTestCase
{
    libCZI::IntRect rect;
    std::uint32_t multiple_of;
    libCZI::IntRect expected_result;
};

struct EnlargeAndLatchTestCaseFixture : public testing::TestWithParam<EnlargeAndLatchTestCase> {};


TEST_P(EnlargeAndLatchTestCaseFixture, EnlargeAndLatchTest)
{
    const auto& test_case = GetParam();

    auto enlarged_rect = PyramidUtilities::EnlargeAndLatch(test_case.rect, test_case.multiple_of);
    EXPECT_EQ(enlarged_rect.x, test_case.expected_result.x);
    EXPECT_EQ(enlarged_rect.y, test_case.expected_result.y);
    EXPECT_EQ(enlarged_rect.w, test_case.expected_result.w);
    EXPECT_EQ(enlarged_rect.h, test_case.expected_result.h);
}

INSTANTIATE_TEST_SUITE_P(
    PyramidUtils,
    EnlargeAndLatchTestCaseFixture,
    testing::Values(
        EnlargeAndLatchTestCase{ { 3, 4, 10, 10 }, 2, { 2, 4, 12, 10 } },
        EnlargeAndLatchTestCase{ { 7, 7, 1, 1 }, 2, { 6, 6, 2, 2 } },
        EnlargeAndLatchTestCase{ { -3, -1, 1, 1 }, 2, { -4, -2, 2, 2 } },
        EnlargeAndLatchTestCase{ { -11, -1, 5, 1 }, 2, { -12, -2, 6, 2 } },
        EnlargeAndLatchTestCase{ { -11, -1, 6, 1 }, 2, { -12, -2, 8, 2 } })
);
