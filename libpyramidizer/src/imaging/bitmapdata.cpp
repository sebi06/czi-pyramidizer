// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "bitmapdata.h"

using namespace std;
using namespace libpyramidizer;

shared_ptr<libCZI::IBitmapData> CBitmapData::CreateBitmapData(libCZI::PixelType pixel_type, std::uint32_t width, std::uint32_t height)
{
    return make_shared<CBitmapData>(pixel_type, width, height);
}
