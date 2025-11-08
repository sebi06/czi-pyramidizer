// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "decimate_opencv.h"
#include <opencv2/imgproc.hpp>

#include <libCZI.h>
#include <gsl/gsl>
#include "bitmapdata.h"
#include "opencvutils.h"

using namespace libpyramidizer;

std::shared_ptr<libCZI::IBitmapData> DecimateOpenCV::Decimate(const std::shared_ptr<libCZI::IBitmapData>& source)
{
    const cv::Size destinationSize(gsl::narrow<int>(source->GetWidth() / 2), gsl::narrow<int>(source->GetHeight() / 2));
    const int open_cv_pixel_type = OpenCVUtils::libCZIPixelTypeToOpenCv(source->GetPixelType());

    auto destination = CBitmapData::CreateBitmapData(source->GetPixelType(), destinationSize.width, destinationSize.height);

    const libCZI::ScopedBitmapLockerSP source_locker(source);
    const libCZI::ScopedBitmapLockerSP destination_locker(destination);
    const cv::Mat source_image(gsl::narrow<int>(source->GetHeight()), gsl::narrow<int>(source->GetWidth()), open_cv_pixel_type, source_locker.ptrDataRoi, source_locker.stride);
    const cv::Mat destination_image(gsl::narrow<int>(destination->GetHeight()), gsl::narrow<int>(destination->GetWidth()), open_cv_pixel_type, destination_locker.ptrDataRoi, destination_locker.stride);

    cv::pyrDown(source_image, destination_image, destinationSize, cv::BORDER_REPLICATE);

    return destination;
}
