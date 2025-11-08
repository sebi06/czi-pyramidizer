// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "commandlineoptions.h"
#include "do_pyramidize.h"
#include "../IConsoleIo.h"
#include <optional>
#include <tuple>
#include <memory>
#include <libCZI.h>

namespace libpyramidizer
{
    class IContext;

    class PyramidizerOperation
    {
    public:
        enum class Result
        {
            NoOperationNecessary,
            OperationSuccessful,
        };

        class IProgressReport
        {
        public:
            IProgressReport() = default;
            virtual ~IProgressReport() = default;

            virtual void ReportProgressPyramidizer(const libCZI::CDimCoordinate& plane_coordinate, std::uint8_t layer, std::uint32_t total_count_of_tiles, std::uint32_t count_of_tiles_done) = 0;

            virtual void ReportProgressCopySubBlock(const libCZI::CDimCoordinate& plane_coordinate, std::uint32_t total_count_of_tiles, std::uint32_t count_of_tiles_done) = 0;

            virtual void ReportProgressCopyAttachment(std::uint32_t total_count_of_attachments, std::uint32_t count_of_attachments_done) = 0;

            // Delete copy/move constructor and copy/move assignment operator
            IProgressReport(const IProgressReport&) = delete;
            IProgressReport& operator=(const IProgressReport&) = delete;
            IProgressReport(IProgressReport&&) = delete;
            IProgressReport& operator=(IProgressReport&&) = delete;
        };

        struct PyramidizerOperationOptions
        {
            /// The command-line options object - attention, this object must outlive the PyramidizerOperation object.
            const CommandLineOptions* command_line_options{ nullptr };

            std::shared_ptr<IConsoleIo> console_io;

            std::shared_ptr<libCZI::IStream> source_stream;

            /// A functor that returns the destination stream object. The functor is called only once at most. This stream
            /// is then be used to write the destination document. The functor must return a valid stream object or throw an
            /// exception if the stream object cannot be created.
            /// Note that the functor may not be called at all - e.g. if an error occurs before, or if it is determined that
            /// no pyramid is necessary.
            std::function<std::shared_ptr<libCZI::IOutputStream>()> get_destination_stream_functor;

            std::shared_ptr<IProgressReport> progress_report;
        };

    private:
        PyramidizerOperationOptions options_;
    public:
        explicit PyramidizerOperation(PyramidizerOperationOptions options);

        Result Execute();

    private:
        const CommandLineOptions& GetCommandLineOptions() const { return *this->options_.command_line_options; }

        void WriteMetadata(const DoPyramidize::PyramidizeResult& pyramidizing_result, const std::shared_ptr<libCZI::IMetadataSegment>& metadata_segment, const std::shared_ptr<libCZI::ICziWriter>& writer);

        void CopyAttachments(const std::shared_ptr<libCZI::ICZIReader>& reader, const std::shared_ptr<libCZI::ICziWriter>& writer);
        void WriteAttachment(const std::shared_ptr<libCZI::ICziWriter>& writer, const std::shared_ptr<libCZI::IAttachment>& attachment);

        std::optional<std::tuple<libCZI::CompressionMode, std::shared_ptr<libCZI::ICompressParameters>>> TryToDetermineCompressionMode(const std::shared_ptr<libCZI::ICZIReader>& reader);
        void PrintOperationalParameters(const std::shared_ptr<libpyramidizer::IContext>& context);

        bool DetermineWhetherBackgroundShouldBeWhite(const std::shared_ptr<libCZI::ICZIReader>& reader);

        bool CheckForNecessityOfPyramid(const std::shared_ptr<libCZI::ICZIReader>& reader);

        void PrintSourceDocumentInfo(const std::shared_ptr<libCZI::ICZIReader>& reader);
        void PrintIntRect(const std::shared_ptr<IConsoleIo>& console_io, const libCZI::IntRect& rect);
        void SetOutputColorForHeader1();
        void SetOutputColorForHeader2();
        void SetOutputColorForValue();
        void SetOutputColorForLabel1();
        void SetOutputColorForLabel2();
        void ResetOutputColor();
    };
} // namespace libpyramidizer
