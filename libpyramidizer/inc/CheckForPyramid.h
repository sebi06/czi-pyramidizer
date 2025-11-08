// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "commandlineoptions.h"
#include <libCZI.h>
#include <memory>
#include <optional>
#include <utility>

namespace libpyramidizer
{
    class CheckForPyramid
    {
    private:
        /// This struct gathers all necessary parameters for making the decision.
        struct ThresholdParameters
        {
            std::uint32_t max_extent_of_image{0};
        };

    public:
        /// Check whether the specified document needs a pyramid. I.e. it is determined whether the document has an extent which
        /// makes is advisable to contain a pyramid. If this is the case, then it is checked whether a pyramid is already present.
        /// In the case of "no pyramid necessary" or "pyramid necessary but already present", false is returned. If a pyramid is necessary
        /// and not already present, true is returned.
        ///
        /// \param  source_stream           A stream containing the source document. A document will be created from this stream, and destroyed after the check.
        /// \param  command_line_options    The command line options (used to get the threshold for the pyramid).
        ///
        /// \returns    True if a pyramid is to be created; false otherwise. I.e. false means - no pyramid is necessary or a pyramid is already present.
        static bool NeedsPyramid(const std::shared_ptr<libCZI::IStream>& source_stream, const CommandLineOptions& command_line_options);

        /// Check whether the specified document needs a pyramid. I.e. it is determined whether the document has an extent which
        /// makes is advisable to contain a pyramid. If this is the case, then it is checked whether a pyramid is already present.
        /// In the case of "no pyramid necessary" or "pyramid necessary but already present", false is returned. If a pyramid is necessary
        /// and not already present, true is returned.
        ///
        /// \param  source_document         The document to be checked.
        /// \param  command_line_options    The command line options (used to get the threshold for the pyramid).
        ///
        /// \returns    True if a pyramid is to be created; false otherwise. I.e. false means - no pyramid is necessary or a pyramid is already present.
        static bool NeedsPyramid(const std::shared_ptr<libCZI::ISubBlockRepository>& source_document, const CommandLineOptions& command_line_options);
    private:
        /// Check overall bounding box for necessity of a pyramid. If the bounding box is larger than a certain threshold, a pyramid is necessary.
        ///
        /// \param  statistics              The statistics.
        /// \param  threshold_parameters    Information describing the threshold.
        ///
        /// \returns    False if the overall bounding box is indicating that no pyramid is necessary; false otherwise.
        static bool CheckOverallBoundingBoxForNecessityOfPyramid(const libCZI::SubBlockStatistics& statistics, const ThresholdParameters& threshold_parameters);


        /// We check the per-scene bounding boxes, and use the threshold parameters to determine if a pyramid is necessary. If false is returned,
        /// then it can be assumed that no pyramid is necessary. True is returned if at least one scene is above the threshold. The nullopt
        /// is returned if there are no scenes.
        ///
        /// \param  statistics              The statistics.
        /// \param  threshold_parameters    Information describing the threshold.
        ///
        /// \returns    False if it is deduced that NO pyramid is necessary, true if a pyramid is necessary; and nullopt if the result is inconclusive.
        static std::optional<bool> CheckPerSceneBoundingBoxesForNecessityOfPyramid(const libCZI::SubBlockStatistics& statistics, const ThresholdParameters& threshold_parameters);

        static bool CheckIfPyramidIsPresent(const libCZI::SubBlockStatistics& statistics, const libCZI::PyramidStatistics& pyramid_statistics, const ThresholdParameters& threshold_parameters);
        static bool IsRectangleAboveThreshold(const libCZI::IntRect& rectangle, const ThresholdParameters& threshold_parameter);
        static bool DoesContainPyramidLayer(const std::vector<libCZI::PyramidStatistics::PyramidLayerStatistics>& pyramid_layer_statistics);
    };
}
