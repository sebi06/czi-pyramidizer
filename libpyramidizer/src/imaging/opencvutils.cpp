// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "opencvutils.h"

#include <opencv2/core.hpp>
#include <opencv2/core/utils/logger.hpp>

using namespace libpyramidizer;

void OpenCVUtils::OneTimeInitializationOpenCv()
{
    // Set the global logging level to warning, which ignores debug messages
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
}

int OpenCVUtils::libCZIPixelTypeToOpenCv(libCZI::PixelType pixelType)
{
    switch (pixelType)
    {
    case libCZI::PixelType::Gray8:
        return CV_8UC1;
    case libCZI::PixelType::Gray16:
        return CV_16UC1;
    case libCZI::PixelType::Bgr24:
        return CV_8UC3;
    case libCZI::PixelType::Bgra32:
        return CV_8UC4;
    default:
        throw std::invalid_argument("Unsupported pixel type");
    }
}
