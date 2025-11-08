// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "commandlineoptions.h"
#include "pyramidizer_operation.h"
#include <memory>
#include <optional>

namespace libpyramidizer
{
    class IDecimationProcessing;

    /// In this interface, we gather all "external" dependencies that are needed by the pyramidizer operation.
    class IContext
    {
    public:
        IContext() = default;

        /// Gets "decimation-processing object" that is used to decimate the images.
        ///
        /// \returns    The "decimation-processing object".
        virtual IDecimationProcessing* GetDecimationProcessing() = 0;

        /// Gets the "command line options" object. Do not store this plain pointer, use it only for querying the options.
        ///
        /// \returns    A plain pointer to the command line options object. 
        virtual const CommandLineOptions* GetCommandLineOptions() = 0;

        /// Gets the "progress reporter object" - this object is used to report progress of the pyramidizer operation.
        /// This may be null if progress reporting is not needed.
        ///
        /// \returns    The progress reporter object.
        virtual std::shared_ptr<PyramidizerOperation::IProgressReport> GetProgressReporter() = 0;

        /// Gets compression mode "determined from source document". This is only retrieved if the user has not specified a 
        /// compression mode on the command line. If the user has specified a compression mode or if the compression mode
        /// could not be determined, this method will return an empty optional.
        ///
        /// \returns    The compression mode determined from source (if available).
        virtual std::optional<std::tuple<libCZI::CompressionMode, std::shared_ptr<libCZI::ICompressParameters>>> GetCompressionModeDeterminedFromSource() = 0;

        /// Gets background color to be used for the specified channel.
        /// Note that currently the channel number is ignored, so it is a global parameter
        /// for the time being.
        ///
        /// \param  channel_number  The channel number.
        ///
        /// \returns    The background color.
        virtual libCZI::RgbFloatColor GetBackgroundColor(int channel_number) = 0;

        virtual ~IContext() = default;

        // Delete copy/move constructor and copy/move assignment operator
        IContext(const IContext&) = delete;
        IContext& operator=(const IContext&) = delete;
        IContext(IContext&&) = delete;
        IContext& operator=(IContext&&) = delete;

        static std::unique_ptr<IContext> CreateUp(
                                            const CommandLineOptions& options, 
                                            const std::shared_ptr<PyramidizerOperation::IProgressReport>& progress_report,
                                            const std::optional<std::tuple<libCZI::CompressionMode, std::shared_ptr<libCZI::ICompressParameters>>>& compression_mode_determined_from_source,
                                            bool background_is_white);
        static std::shared_ptr<IContext> CreateSp(
                                            const CommandLineOptions& options, 
                                            const std::shared_ptr<PyramidizerOperation::IProgressReport>& progress_report,
                                            const std::optional<std::tuple<libCZI::CompressionMode, std::shared_ptr<libCZI::ICompressParameters>>>& compression_mode_determined_from_source,
                                            bool background_is_white);
    };
} // namespace libpyramidizer
