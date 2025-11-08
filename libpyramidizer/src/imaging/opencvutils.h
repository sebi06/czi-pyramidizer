// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>

namespace libpyramidizer
{
    class OpenCVUtils
    {
    public:
        static int libCZIPixelTypeToOpenCv(libCZI::PixelType pixelType);

        static void OneTimeInitializationOpenCv();
    };
} // namespace libpyramidizer
