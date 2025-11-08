// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>

#include <memory>
#include <functional>

namespace libpyramidizer
{
    class ComposeBitmaps
    {
    public:
        static std::shared_ptr<libCZI::IBitmapData> Compose(
            const libCZI::IntRect& roi,
            libCZI::PixelType pixel_type,
            const libCZI::RgbFloatColor& background_color,
            const std::function<bool(std::shared_ptr<libCZI::IBitmapData>&, int&, int&)>& get_bitmap);
    private:
        static void CopyAt(
            const std::shared_ptr<libCZI::IBitmapData>& destination,
            const std::shared_ptr<libCZI::IBitmapData>& source,
            int x_offset,
            int y_offset);
        static void Fill(const std::shared_ptr<libCZI::IBitmapData>& bitmap, const libCZI::RgbFloatColor& color);
    };
} // namespace libpyramidizer
