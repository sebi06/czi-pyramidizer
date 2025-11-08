// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include "src/imaging/compose.h"
#include "src/imaging/bitmapdata.h"

using namespace std;
using namespace libCZI;
using namespace libpyramidizer;

TEST(ImagingTests, ComposeScenario1)
{
    static constexpr uint8_t source1[] =
    {
        0x01,0x02,
        0x03,0x04
    };

    static constexpr uint8_t source2[] =
    {
        0x11,0x12,
        0x13,0x14
    };

    auto bitmap1 = make_shared<CBitmapData>(PixelType::Gray8, 2, 2, source1, 2);
    auto bitmap2 = make_shared<CBitmapData>(PixelType::Gray8, 2, 2, source2, 2);

    int counter = 0;
    auto composed = ComposeBitmaps::Compose(
        IntRect{ 0, 0, 4, 2 },
        PixelType::Gray8,
        RgbFloatColor{ 0.0f, 0.0f, 0.0f },
        [&](shared_ptr<IBitmapData>& bitmap, int& x, int& y) -> bool
        {
            if (counter == 0)
            {
                bitmap = bitmap1;
                x = 0;
                y = 0;
            }
            else if (counter == 1)
            {
                bitmap = bitmap2;
                x = 2;
                y = 0;
            }
            else
            {
                return false;
            }

            ++counter;
            return true;
        });

    const auto composed_locker = ScopedBitmapLockerSP(composed);

    const uint8_t* data = static_cast<const uint8_t*>(composed_locker.ptrDataRoi);
    ASSERT_EQ(0x01, data[0]);
    ASSERT_EQ(0x02, data[1]);
    ASSERT_EQ(0x11, data[2]);
    ASSERT_EQ(0x12, data[3]);

    ASSERT_EQ(0x03, data[0 + composed_locker.stride]);
    ASSERT_EQ(0x04, data[1 + composed_locker.stride]);
    ASSERT_EQ(0x13, data[2 + composed_locker.stride]);
    ASSERT_EQ(0x14, data[3 + composed_locker.stride]);
}

TEST(ImagingTests, ComposeScenario2)
{
    static constexpr uint8_t source1[] =
    {
        0x01,0x02,03,0x04,
        0x05,0x06,07,0x08,
    };

    static constexpr uint8_t source2[] =
    {
        0x11,0x12,
        0x13,0x14
    };

    auto bitmap1 = make_shared<CBitmapData>(PixelType::Gray8, 4, 2, source1, 4);
    auto bitmap2 = make_shared<CBitmapData>(PixelType::Gray8, 2, 2, source2, 2);

    int counter = 0;
    auto composed = ComposeBitmaps::Compose(
        IntRect{ 0, 0, 4, 2 },
        PixelType::Gray8,
        RgbFloatColor{ 0.0f, 0.0f, 0.0f },
        [&](shared_ptr<IBitmapData>& bitmap, int& x, int& y) -> bool
        {
            if (counter == 0)
            {
                bitmap = bitmap1;
                x = 0;
                y = 0;
            }
            else if (counter == 1)
            {
                bitmap = bitmap2;
                x = 3;
                y = 0;
            }
            else
            {
                return false;
            }

            ++counter;
            return true;
        });

    const auto composed_locker = ScopedBitmapLockerSP(composed);

    const uint8_t* data = static_cast<const uint8_t*>(composed_locker.ptrDataRoi);
    ASSERT_EQ(0x01, data[0]);
    ASSERT_EQ(0x02, data[1]);
    ASSERT_EQ(0x03, data[2]);
    ASSERT_EQ(0x11, data[3]);

    ASSERT_EQ(0x05, data[0 + composed_locker.stride]);
    ASSERT_EQ(0x06, data[1 + composed_locker.stride]);
    ASSERT_EQ(0x07, data[2 + composed_locker.stride]);
    ASSERT_EQ(0x13, data[3 + composed_locker.stride]);
}

TEST(ImagingTests, ComposeScenario3)
{
    static constexpr uint8_t source1[] =
    {
        0x01,0x02,03,0x04,
        0x05,0x06,07,0x08,
    };

    static constexpr uint8_t source2[] =
    {
        0x11,0x12,
        0x13,0x14
    };

    auto bitmap1 = make_shared<CBitmapData>(PixelType::Gray8, 4, 2, source1, 4);
    auto bitmap2 = make_shared<CBitmapData>(PixelType::Gray8, 2, 2, source2, 2);

    int counter = 0;
    auto composed = ComposeBitmaps::Compose(
        IntRect{ 0, 0, 4, 2 },
        PixelType::Gray8,
        RgbFloatColor{ 0.0f, 0.0f, 0.0f },
        [&](shared_ptr<IBitmapData>& bitmap, int& x, int& y) -> bool
        {
            if (counter == 0)
            {
                bitmap = bitmap1;
                x = 0;
                y = 0;
            }
            else if (counter == 1)
            {
                bitmap = bitmap2;
                x = 1;
                y = 1;
            }
            else
            {
                return false;
            }

            ++counter;
            return true;
        });

    const auto composed_locker = ScopedBitmapLockerSP(composed);

    const uint8_t* data = static_cast<const uint8_t*>(composed_locker.ptrDataRoi);
    ASSERT_EQ(0x01, data[0]);
    ASSERT_EQ(0x02, data[1]);
    ASSERT_EQ(0x03, data[2]);
    ASSERT_EQ(0x04, data[3]);

    ASSERT_EQ(0x05, data[0 + composed_locker.stride]);
    ASSERT_EQ(0x11, data[1 + composed_locker.stride]);
    ASSERT_EQ(0x12, data[2 + composed_locker.stride]);
    ASSERT_EQ(0x08, data[3 + composed_locker.stride]);
}

TEST(ImagingTests, ComposeScenario4)
{
    static constexpr uint8_t source1[] =
    {
        0x01,0x02,03,0x04,
        0x05,0x06,07,0x08,
    };

    static constexpr uint8_t source2[] =
    {
        0x11,0x12,
        0x13,0x14
    };

    auto bitmap1 = make_shared<CBitmapData>(PixelType::Gray8, 4, 2, source1, 4);
    auto bitmap2 = make_shared<CBitmapData>(PixelType::Gray8, 2, 2, source2, 2);

    int counter = 0;
    auto composed = ComposeBitmaps::Compose(
        IntRect{ 0, 0, 4, 2 },
        PixelType::Gray8,
        RgbFloatColor{ 0.0f, 0.0f, 0.0f },
        [&](shared_ptr<IBitmapData>& bitmap, int& x, int& y) -> bool
        {
            if (counter == 0)
            {
                bitmap = bitmap1;
                x = 0;
                y = 0;
            }
            else if (counter == 1)
            {
                bitmap = bitmap2;
                x = 1;
                y = -1;
            }
            else
            {
                return false;
            }

            ++counter;
            return true;
        });

    const auto composed_locker = ScopedBitmapLockerSP(composed);

    const uint8_t* data = static_cast<const uint8_t*>(composed_locker.ptrDataRoi);
    ASSERT_EQ(0x01, data[0]);
    ASSERT_EQ(0x13, data[1]);
    ASSERT_EQ(0x14, data[2]);
    ASSERT_EQ(0x04, data[3]);

    ASSERT_EQ(0x05, data[0 + composed_locker.stride]);
    ASSERT_EQ(0x06, data[1 + composed_locker.stride]);
    ASSERT_EQ(0x07, data[2 + composed_locker.stride]);
    ASSERT_EQ(0x08, data[3 + composed_locker.stride]);
}

TEST(ImagingTests, ComposeScenario5)
{
    static constexpr uint8_t source1[] =
    {
        0x01,0x02,03,0x04,
        0x05,0x06,07,0x08,
    };

    static constexpr uint8_t source2[] =
    {
        0x11,0x12,
        0x13,0x14
    };

    auto bitmap1 = make_shared<CBitmapData>(PixelType::Gray8, 4, 2, source1, 4);
    auto bitmap2 = make_shared<CBitmapData>(PixelType::Gray8, 2, 2, source2, 2);

    int counter = 0;
    auto composed = ComposeBitmaps::Compose(
        IntRect{ 0, 0, 4, 2 },
        PixelType::Gray8,
        RgbFloatColor{ 0.0f, 0.0f, 0.0f },
        [&](shared_ptr<IBitmapData>& bitmap, int& x, int& y) -> bool
        {
            if (counter == 0)
            {
                bitmap = bitmap1;
                x = 0;
                y = 0;
            }
            else if (counter == 1)
            {
                bitmap = bitmap2;
                x = -1;
                y = 0;
            }
            else
            {
                return false;
            }

            ++counter;
            return true;
        });

    const auto composed_locker = ScopedBitmapLockerSP(composed);

    const uint8_t* data = static_cast<const uint8_t*>(composed_locker.ptrDataRoi);
    ASSERT_EQ(0x12, data[0]);
    ASSERT_EQ(0x02, data[1]);
    ASSERT_EQ(0x03, data[2]);
    ASSERT_EQ(0x04, data[3]);

    ASSERT_EQ(0x14, data[0 + composed_locker.stride]);
    ASSERT_EQ(0x06, data[1 + composed_locker.stride]);
    ASSERT_EQ(0x07, data[2 + composed_locker.stride]);
    ASSERT_EQ(0x08, data[3 + composed_locker.stride]);
}
