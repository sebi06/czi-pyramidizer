// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include "inc/tile_helper.h"
#include <array>

using namespace std;
using namespace libpyramidizer;

TEST(TileHelperTest, GetMinimalRectForRoiTestCase1)
{
    TileHelper tileHelper;
    static constexpr std::array<libCZI::IntRect, 2> kRects = { {{ 0, 0, 10, 10 },  { 10, 10, 10, 10 }} };
    tileHelper.Initialize(kRects);
    auto minimalRect = tileHelper.GetMinimalRectForRoi(libCZI::IntRect{ 15,5,10,10 });
    EXPECT_EQ(minimalRect.x, 15);
    EXPECT_EQ(minimalRect.y, 10);
    EXPECT_EQ(minimalRect.w, 5);
    EXPECT_EQ(minimalRect.h, 5);
}

TEST(TileHelperTest, GetMinimalRectForRoiTestCase2)
{
    TileHelper tileHelper;
    static constexpr std::array<libCZI::IntRect, 2> kRects = { {{ 0, 0, 10, 10 },  { 10, 10, 10, 10 }} };
    tileHelper.Initialize(kRects);
    auto minimalRect = tileHelper.GetMinimalRectForRoi(libCZI::IntRect{ 15,0,10,10 });
    EXPECT_FALSE(minimalRect.IsNonEmpty());
}

TEST(TileHelperTest, GetMinimalRectForRoiTestCase3)
{
    TileHelper tileHelper;
    static constexpr std::array<libCZI::IntRect, 2> kRects = { {{ 0, 0, 10, 10 },  { 10, 10, 10, 10 }} };
    tileHelper.Initialize(kRects);
    auto minimalRect = tileHelper.GetMinimalRectForRoi(libCZI::IntRect{ 5, 5, 10, 10 });
    EXPECT_EQ(minimalRect.x, 5);
    EXPECT_EQ(minimalRect.y, 5);
    EXPECT_EQ(minimalRect.w, 10);
    EXPECT_EQ(minimalRect.h, 10);
}
