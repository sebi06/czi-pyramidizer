// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "compose.h"

#include "bitmapdata.h"
#include <cmath>
#include <gsl/gsl>

using namespace libCZI;
using namespace std;
using namespace libpyramidizer;

namespace
{
    std::uint8_t clampToByte(float f)
    {
        if (f <= 0)
        {
            return 0;
        }
        else if (f >= 255)
        {
            return 255;
        }

        return static_cast<std::uint8_t>(f + .5f);
    }

    std::uint16_t clampToUShort(float f)
    {
        if (f <= 0)
        {
            return 0;
        }
        else if (f >= 65535)
        {
            return 65535;
        }

        return static_cast<std::uint16_t>(f + .5f);
    }
}

/*static*/std::shared_ptr<libCZI::IBitmapData> ComposeBitmaps::Compose(
    const libCZI::IntRect& roi,
    libCZI::PixelType pixel_type,
    const libCZI::RgbFloatColor& background_color,
    const std::function<bool(std::shared_ptr<libCZI::IBitmapData>&, int& x, int& y)>& get_bitmap)
{
    auto destination = CBitmapData::CreateBitmapData(pixel_type, roi.w, roi.h);
    if (!isnan(background_color.r) && !isnan(background_color.g) && !isnan(background_color.b))
    {
        ComposeBitmaps::Fill(destination, background_color);
    }

    for (;;)
    {
        shared_ptr<libCZI::IBitmapData> bitmap;
        int x, y;
        const bool valid = get_bitmap(bitmap, x, y);
        if (!valid)
        {
            break;
        }

        ComposeBitmaps::CopyAt(destination, bitmap, x, y);
    }

    return destination;
}

/*static*/void ComposeBitmaps::CopyAt(
    const std::shared_ptr<libCZI::IBitmapData>& destination,
    const std::shared_ptr<libCZI::IBitmapData>& source,
    int x_offset,
    int y_offset)
{
    const auto source_roi = IntRect{ x_offset, y_offset, gsl::narrow<int>(source->GetWidth()), gsl::narrow<int>(source->GetHeight()) };
    const auto destination_roi = IntRect{ 0, 0, gsl::narrow<int>(destination->GetWidth()), gsl::narrow<int>(destination->GetHeight()) };

    const auto intersection = source_roi.Intersect(destination_roi);
    if (intersection.IsNonEmpty())
    {
        const auto source_locker = ScopedBitmapLockerSP(source);
        const auto destination_locker = ScopedBitmapLockerSP(destination);

        const uint8_t bytes_per_pel = libCZI::Utils::GetBytesPerPixel(source->GetPixelType());

        void* ptrDestination = static_cast<char*>(destination_locker.ptrDataRoi) +
            intersection.y * static_cast<size_t>(destination_locker.stride) +
            intersection.x * static_cast<size_t>(bytes_per_pel);
        const void* ptrSource = static_cast<const char*>(source_locker.ptrDataRoi) +
            (std::max)(-y_offset, 0) * static_cast<size_t>(source_locker.stride) +
            (std::max)(-x_offset, 0) * static_cast<size_t>(bytes_per_pel);

        const size_t line_length = static_cast<size_t>(bytes_per_pel) * intersection.w;
        for (int y = 0; y < intersection.h; ++y)
        {
            memcpy(ptrDestination, ptrSource, line_length);
            ptrDestination = static_cast<char*>(ptrDestination) + destination_locker.stride;
            ptrSource = static_cast<const char*>(ptrSource) + source_locker.stride;
        }
    }
}

/*static*/void ComposeBitmaps::Fill(const std::shared_ptr<libCZI::IBitmapData>& bitmap, const libCZI::RgbFloatColor& color)
{
    ScopedBitmapLockerSP bitmap_locker{ bitmap };

    switch (bitmap->GetPixelType())
    {
        case PixelType::Gray8:
        {
            const uint8_t value = clampToByte((255 * (color.r + color.g + color.b)) / 3);
            for (uint32_t y = 0; y < bitmap->GetHeight(); ++y)
            {
                void* p = static_cast<char*>(bitmap_locker.ptrDataRoi) + (y * static_cast<ptrdiff_t>(bitmap_locker.stride));
                memset(p, value, bitmap->GetWidth());
            }

            break;
        }
        case PixelType::Gray16:
        {
            const uint16_t value = clampToUShort((65535 * (color.r + color.g + color.b)) / 3);
            for (uint32_t y = 0; y < bitmap->GetHeight(); ++y)
            {
                std::uint16_t* p = reinterpret_cast<std::uint16_t*>(static_cast<char*>(bitmap_locker.ptrDataRoi) + (y * static_cast<ptrdiff_t>(bitmap_locker.stride)));
                for (uint32_t x = 0; x < bitmap->GetWidth(); ++x)
                {
                    *(p + x) = value;
                }
            }

            break;
        }
        case PixelType::Gray32Float:
        {
            const float value = (color.r + color.g + color.b) / 3;
            for (uint32_t y = 0; y < bitmap->GetHeight(); ++y)
            {
                float* p = reinterpret_cast<float*>(static_cast<char*>(bitmap_locker.ptrDataRoi) + (y * static_cast<ptrdiff_t>(bitmap_locker.stride)));
                for (uint32_t x = 0; x < bitmap->GetWidth(); ++x)
                {
                    *(p + x) = value;
                }
            }

            break;
        }
        case PixelType::Bgr24:
        {
            const uint8_t value_red = clampToByte(255 * color.r);
            const uint8_t value_green = clampToByte(255 * color.g);
            const uint8_t value_blue = clampToByte(255 * color.b);

            for (uint32_t y = 0; y < bitmap->GetHeight(); ++y)
            {
                std::uint8_t* p = reinterpret_cast<std::uint8_t*>(static_cast<char*>(bitmap_locker.ptrDataRoi) + (y * static_cast<ptrdiff_t>(bitmap_locker.stride)));
                for (uint32_t x = 0; x < bitmap->GetWidth(); ++x)
                {
                    *p++ = value_blue;
                    *p++ = value_green;
                    *p++ = value_red;
                }
            }

            break;
        }
        case PixelType::Bgr48:
        {
            const uint16_t value_red = clampToUShort(65535 * color.r);
            const uint16_t value_green = clampToUShort(65535 * color.g);
            const uint16_t value_blue = clampToUShort(65535 * color.b);

            for (uint32_t y = 0; y < bitmap->GetHeight(); ++y)
            {
                std::uint16_t* p = reinterpret_cast<std::uint16_t*>(static_cast<char*>(bitmap_locker.ptrDataRoi) + (y * static_cast<ptrdiff_t>(bitmap_locker.stride)));
                for (uint32_t x = 0; x < bitmap->GetWidth(); ++x)
                {
                    *p++ = value_blue;
                    *p++ = value_green;
                    *p++ = value_red;
                }
            }

            break;
        }
        default:
            throw runtime_error("Unsupported pixel type");
    }
}
