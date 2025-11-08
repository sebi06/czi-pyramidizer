// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>
#include <memory>

namespace libpyramidizer
{
    /// In this interface image-processing operations related to the pyramid-generation are gathered.
    class IDecimationProcessing
    {
    public:
        IDecimationProcessing() = default;

        virtual std::shared_ptr<libCZI::IBitmapData> Decimate(const std::shared_ptr<libCZI::IBitmapData>& bitmap) = 0;

        virtual ~IDecimationProcessing() = default;

        // Non-copyable and non-moveable
        IDecimationProcessing(const IDecimationProcessing&) = delete;
        IDecimationProcessing& operator=(const IDecimationProcessing&) = delete;
        IDecimationProcessing(IDecimationProcessing&&) = delete;
        IDecimationProcessing& operator=(IDecimationProcessing&&) = delete;
    };

    IDecimationProcessing* GetDecimationProcessing();
} // namespace libpyramidizer
