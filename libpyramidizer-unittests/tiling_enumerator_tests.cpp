// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include "inc/tiling_enumerator.h"
#include <array>

using namespace std;
using namespace libCZI;
using namespace libpyramidizer;

namespace
{
    bool compare_for_equality(const IntRect& a, const IntRect& b)
    {
        return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
    }
}

TEST(TilingEnumeratorTest, EnumerateTilesScenario1)
{
    TilingEnumerator tiling_enumerator({ 0, 0, 10, 10 }, 5, 5);

    array<IntRect, 4> expected_tiles = { IntRect{ 0, 0, 5, 5 }, IntRect{ 5, 0, 5, 5 }, IntRect{ 0, 5, 5, 5 }, IntRect{ 5, 5, 5, 5 } };
    uint32_t count = 0;
    for (auto roi : tiling_enumerator)
    {
        ASSERT_LT(count, expected_tiles.size());
        EXPECT_TRUE(compare_for_equality(roi, expected_tiles[count]));
        ++count;
    }
}

TEST(TilingEnumeratorTest, EnumerateTilesScenario2)
{
    TilingEnumerator tiling_enumerator({ 0, 0, 11, 10 }, 5, 5);

    array<IntRect, 6> expected_tiles = 
    {
        IntRect{ 0, 0, 5, 5 }, IntRect{ 5, 0, 5, 5 }, IntRect{ 10, 0, 1, 5 },
        IntRect{ 0, 5, 5, 5 }, IntRect{ 5, 5, 5, 5 }, IntRect{ 10, 5, 1, 5 }
    };

    uint32_t count = 0;
    for (auto roi : tiling_enumerator)
    {
        ASSERT_LT(count, expected_tiles.size());
        EXPECT_TRUE(compare_for_equality(roi, expected_tiles[count]));
        ++count;
    }
}

TEST(TilingEnumeratorTest, EnumerateTilesScenario3)
{
    TilingEnumerator tiling_enumerator({ 0, 0, 11, 11 }, 5, 5);

    array<IntRect, 9> expected_tiles =
    {
        IntRect{ 0, 0, 5, 5 }, IntRect{ 5, 0, 5, 5 }, IntRect{ 10, 0, 1, 5 },
        IntRect{ 0, 5, 5, 5 }, IntRect{ 5, 5, 5, 5 }, IntRect{ 10, 5, 1, 5 },
        IntRect{ 0, 10, 5, 1 }, IntRect{ 5, 10, 5, 1 }, IntRect{ 10, 10, 1, 1 }
    };

    uint32_t count = 0;
    for (auto roi : tiling_enumerator)
    {
        ASSERT_LT(count, expected_tiles.size());
        EXPECT_TRUE(compare_for_equality(roi, expected_tiles[count]));
        ++count;
    }
}

TEST(TilingEnumeratorTest, EnumerateTilesScenario4)
{
    TilingEnumerator tiling_enumerator({ -10, -10, 10, 10 }, 5, 5);

    array<IntRect, 4> expected_tiles = { IntRect{ -10, -10, 5, 5 }, IntRect{ -5, -10, 5, 5 }, IntRect{ -10, -5, 5, 5 }, IntRect{ -5, -5, 5, 5 } };
    uint32_t count = 0;
    for (auto roi : tiling_enumerator)
    {
        ASSERT_LT(count, expected_tiles.size());
        EXPECT_TRUE(compare_for_equality(roi, expected_tiles[count]));
        ++count;
    }
}
