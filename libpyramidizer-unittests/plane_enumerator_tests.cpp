// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include "inc/plane_enumerator.h"
#include <array>
#include <vector>
#include <exception>

using namespace std;
using namespace libCZI;
using namespace libpyramidizer;

namespace
{
    bool Contains(const vector<CDimCoordinate>& regions, const CDimCoordinate& region)
    {
        return find_if(regions.begin(), regions.end(), [&region](const CDimCoordinate& r) { return Utils::Compare(&r, &region) == 0; }) != regions.end();
    }
}

TEST(PlaneEnumeratorTest, EnumeratePlanesTestCase1)
{
    SubBlockStatistics sub_block_statistics;
    sub_block_statistics.Invalidate();
    sub_block_statistics.dimBounds = CDimBounds{ { DimensionIndex::C, 0, 2 }, { DimensionIndex::T, 0, 3 } };
    sub_block_statistics.boundingBoxLayer0Only = IntRect{ 0, 0, 10, 10 };

    PlaneEnumerator planeEnumerator(sub_block_statistics);
    vector<CDimCoordinate> regions;
    for (auto it = planeEnumerator.begin(); it != planeEnumerator.end(); ++it)
    {
        const auto region = *it;
        EXPECT_EQ(region.rect.x, sub_block_statistics.boundingBoxLayer0Only.x);
        EXPECT_EQ(region.rect.y, sub_block_statistics.boundingBoxLayer0Only.y);
        EXPECT_EQ(region.rect.w, sub_block_statistics.boundingBoxLayer0Only.w);
        EXPECT_EQ(region.rect.h, sub_block_statistics.boundingBoxLayer0Only.h);
        regions.push_back(region.plane_coordinate);
    }

    EXPECT_EQ(regions.size(), 6);
    EXPECT_TRUE(Contains(regions, CDimCoordinate{ { DimensionIndex::C, 0},  { DimensionIndex::T, 0} }));
    EXPECT_TRUE(Contains(regions, CDimCoordinate{ { DimensionIndex::C, 1},  { DimensionIndex::T, 0} }));
    EXPECT_TRUE(Contains(regions, CDimCoordinate{ { DimensionIndex::C, 0},  { DimensionIndex::T, 1} }));
    EXPECT_TRUE(Contains(regions, CDimCoordinate{ { DimensionIndex::C, 1},  { DimensionIndex::T, 1} }));
    EXPECT_TRUE(Contains(regions, CDimCoordinate{ { DimensionIndex::C, 0},  { DimensionIndex::T, 2} }));
    EXPECT_TRUE(Contains(regions, CDimCoordinate{ { DimensionIndex::C, 1},  { DimensionIndex::T, 2} }));
}

TEST(PlaneEnumeratorTest, EnumeratePlanesTestCase2)
{
    SubBlockStatistics sub_block_statistics;
    sub_block_statistics.Invalidate();
    sub_block_statistics.dimBounds = CDimBounds{ { DimensionIndex::C, 0, 2 }, { DimensionIndex::S, 0, 2 } };
    sub_block_statistics.boundingBoxLayer0Only = IntRect{ 0, 0, 10, 10 };
    sub_block_statistics.sceneBoundingBoxes[0] = { IntRect{ 0, 0, 3, 3 }, IntRect{ 0, 0, 4, 4 } };
    sub_block_statistics.sceneBoundingBoxes[1] = { IntRect{ 5, 5, 3, 3 }, IntRect{ 5, 5, 4, 4 } };

    PlaneEnumerator planeEnumerator(sub_block_statistics);
    vector<CDimCoordinate> regions;
    for (auto it = planeEnumerator.begin(); it != planeEnumerator.end(); ++it)
    {
        const auto region = *it;

        int s_index;
        EXPECT_TRUE(region.plane_coordinate.TryGetPosition(DimensionIndex::S, &s_index));
        EXPECT_EQ(region.rect.x, sub_block_statistics.sceneBoundingBoxes.at(s_index).boundingBoxLayer0.x);
        EXPECT_EQ(region.rect.y, sub_block_statistics.sceneBoundingBoxes.at(s_index).boundingBoxLayer0.y);
        EXPECT_EQ(region.rect.w, sub_block_statistics.sceneBoundingBoxes.at(s_index).boundingBoxLayer0.w);
        EXPECT_EQ(region.rect.h, sub_block_statistics.sceneBoundingBoxes.at(s_index).boundingBoxLayer0.h);
        regions.push_back(region.plane_coordinate);
    }

    EXPECT_EQ(regions.size(), 4);
    EXPECT_TRUE(Contains(regions, CDimCoordinate{ { DimensionIndex::C, 0},  { DimensionIndex::S, 0} }));
    EXPECT_TRUE(Contains(regions, CDimCoordinate{ { DimensionIndex::C, 1},  { DimensionIndex::S, 0} }));
    EXPECT_TRUE(Contains(regions, CDimCoordinate{ { DimensionIndex::C, 0},  { DimensionIndex::S, 1} }));
    EXPECT_TRUE(Contains(regions, CDimCoordinate{ { DimensionIndex::C, 1},  { DimensionIndex::S, 1} }));
}

TEST(PlaneEnumeratorTest, UseInvalidSubBlockStatisticsAndExpectException1)
{
    SubBlockStatistics sub_block_statistics;
    sub_block_statistics.Invalidate();
    sub_block_statistics.dimBounds = CDimBounds{ { DimensionIndex::S, 0, 2 } };
    sub_block_statistics.sceneBoundingBoxes[0] = { IntRect{ 0, 0, 3, 3 }, IntRect{ 0, 0, 4, 4 } };
    sub_block_statistics.sceneBoundingBoxes[1] = { IntRect{ 5, 5, 3, 3 }, IntRect{ 5, 5, 4, 4 } };

    EXPECT_THROW(PlaneEnumerator planeEnumerator(sub_block_statistics), invalid_argument);
}

TEST(PlaneEnumeratorTest, UseInvalidSubBlockStatisticsAndExpectException2)
{
    SubBlockStatistics sub_block_statistics;
    sub_block_statistics.Invalidate();
    sub_block_statistics.dimBounds = CDimBounds{ { DimensionIndex::S, 0, 2 }, {DimensionIndex::T, 0,1} };
    sub_block_statistics.sceneBoundingBoxes[0] = { IntRect{ 0, 0, 3, 3 }, IntRect{ 0, 0, 4, 4 } };

    EXPECT_THROW(PlaneEnumerator planeEnumerator(sub_block_statistics), invalid_argument);
}
