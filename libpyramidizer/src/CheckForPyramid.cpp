// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "../inc/CheckForPyramid.h"

#include "gsl/narrow"

using namespace std;
using namespace libCZI;
using namespace libpyramidizer;

/*static*/bool CheckForPyramid::NeedsPyramid(const std::shared_ptr<libCZI::IStream>& source_stream, const CommandLineOptions& command_line_options)
{
    const auto reader = libCZI::CreateCZIReader();
    reader->Open(source_stream);
    return CheckForPyramid::NeedsPyramid(reader, command_line_options);
}

/*static*/bool CheckForPyramid::NeedsPyramid(const std::shared_ptr<libCZI::ISubBlockRepository>& source_document, const CommandLineOptions& command_line_options)
{
    ThresholdParameters parameters{ command_line_options.GetThresholdWhenPyramidIsRequired() };

    const auto statistics = source_document->GetStatistics();

    // first, check the "overall bounding box" for necessity of a pyramid
    if (!CheckForPyramid::CheckOverallBoundingBoxForNecessityOfPyramid(statistics, parameters))
    {
        // if this returns false, we are sure that no pyramid is needed
        return false;
    }

    // ...now, check the per-scene bounding boxes for necessity of a pyramid
    const auto per_scene_result = CheckForPyramid::CheckPerSceneBoundingBoxesForNecessityOfPyramid(statistics, parameters);
    if (per_scene_result.value_or(true) == false)
    {
        // if this returns false, we are sure that no pyramid is needed
        return false;
    }

    // ok, now we are sure that we need a pyramid, now let's check if it is already present
    const auto pyramid_statistics = source_document->GetPyramidStatistics();

    if (CheckForPyramid::CheckIfPyramidIsPresent(statistics, pyramid_statistics, parameters))
    {
        return false;
    }

    return true;
}

/*static*/bool CheckForPyramid::CheckOverallBoundingBoxForNecessityOfPyramid(const libCZI::SubBlockStatistics& statistics, const ThresholdParameters& threshold_parameters)
{
    if (CheckForPyramid::IsRectangleAboveThreshold(statistics.boundingBoxLayer0Only, threshold_parameters))
    {
        return true;
    }

    return false;
}

/*static*/optional<bool> CheckForPyramid::CheckPerSceneBoundingBoxesForNecessityOfPyramid(const libCZI::SubBlockStatistics& statistics, const ThresholdParameters& threshold_parameters)
{
    if (statistics.sceneBoundingBoxes.empty())
    {
        return nullopt;
    }

    for (const auto& sceneBoundingBox : statistics.sceneBoundingBoxes)
    {
        if (CheckForPyramid::IsRectangleAboveThreshold(sceneBoundingBox.second.boundingBoxLayer0, threshold_parameters))
        {
            return true;
        }
    }

    return false;
}

/*static*/bool CheckForPyramid::IsRectangleAboveThreshold(const libCZI::IntRect& rectangle, const ThresholdParameters& threshold_parameters)
{
    if (gsl::narrow<uint32_t>(rectangle.w) > threshold_parameters.max_extent_of_image ||
        gsl::narrow<uint32_t>(rectangle.h) > threshold_parameters.max_extent_of_image)
    {
        return true;
    }

    return false;
}

/*static*/bool CheckForPyramid::CheckIfPyramidIsPresent(const libCZI::SubBlockStatistics& statistics, const libCZI::PyramidStatistics& pyramid_statistics, const ThresholdParameters& threshold_parameters)
{
    if (statistics.sceneBoundingBoxes.empty())
    {
        // this means - no S-index used, so we only have to deal with the overall bounding box (boundingBoxLayer0Only)
        if (CheckForPyramid::CheckOverallBoundingBoxForNecessityOfPyramid(statistics, threshold_parameters))
        {
            const auto& pyramid_layer_statistics_iterator = pyramid_statistics.scenePyramidStatistics.find(numeric_limits<int>::max());
            if (pyramid_layer_statistics_iterator == pyramid_statistics.scenePyramidStatistics.end())
            {
                // well, this is not expected... there should always be a pyramid-layer-0
                return false;
            }

            if (!CheckForPyramid::DoesContainPyramidLayer(pyramid_layer_statistics_iterator->second))
            {
                return false;
            }
        }
    }
    else
    {
        // now this means - the document contains "scenes" or an "S-dimension" - so we have to check the per-scene bounding boxes
        if (CheckForPyramid::CheckPerSceneBoundingBoxesForNecessityOfPyramid(statistics, threshold_parameters))
        {
            for (const auto& sceneBoundingBox : statistics.sceneBoundingBoxes)
            {
                if (CheckForPyramid::IsRectangleAboveThreshold(sceneBoundingBox.second.boundingBoxLayer0, threshold_parameters))
                {
                    const auto& pyramid_layer_statistics_iterator = pyramid_statistics.scenePyramidStatistics.find(sceneBoundingBox.first);
                    if (pyramid_layer_statistics_iterator == pyramid_statistics.scenePyramidStatistics.end())
                    {
                        // well, this is not expected... there should always be a pyramid-layer-0
                        return false;
                    }

                    if (!CheckForPyramid::DoesContainPyramidLayer(pyramid_layer_statistics_iterator->second))
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

/*static*/bool CheckForPyramid::DoesContainPyramidLayer(const std::vector<libCZI::PyramidStatistics::PyramidLayerStatistics>& pyramid_layer_statistics)
{
    for (const auto& layer_statistics : pyramid_layer_statistics)
    {
        if (!layer_statistics.layerInfo.IsLayer0() && layer_statistics.count > 0)
        {
            return true;
        }
    }

    return false;
}
