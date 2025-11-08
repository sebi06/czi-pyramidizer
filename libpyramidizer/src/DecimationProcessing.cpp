// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "../inc/IDecimationProcessing.h"
#include "imaging/decimate_opencv.h"
#include "imaging/opencvutils.h"

using namespace libpyramidizer;

class DecimationProcessing : public IDecimationProcessing
{
public:
    DecimationProcessing()
    {
        OpenCVUtils::OneTimeInitializationOpenCv();
    }

    std::shared_ptr<libCZI::IBitmapData> Decimate(const std::shared_ptr<libCZI::IBitmapData>& bitmap) override
    {
        return DecimateOpenCV::Decimate(bitmap);
    }
};

namespace
{
    DecimationProcessing g_decimationProcessing;
}

IDecimationProcessing* libpyramidizer::GetDecimationProcessing()
{
    return &g_decimationProcessing;
}
