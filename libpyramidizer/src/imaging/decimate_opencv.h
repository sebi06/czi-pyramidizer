// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>
#include <memory>

namespace libpyramidizer
{
    class DecimateOpenCV
    {
    public:
        static std::shared_ptr<libCZI::IBitmapData> Decimate(const std::shared_ptr<libCZI::IBitmapData>& source);
    };
} // namespace libpyramidizer
