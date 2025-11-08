// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "../inc/IContext.h"
#include "../inc/IDecimationProcessing.h"
#include <memory>

using namespace libpyramidizer;

class Context : public IContext
{
private:
    const CommandLineOptions& command_line_options_;
    std::shared_ptr<PyramidizerOperation::IProgressReport> progress_report_;
    std::optional<std::tuple<libCZI::CompressionMode, std::shared_ptr<libCZI::ICompressParameters>>> compression_mode_determined_from_source_;
    bool background_is_white_;
public:
    Context() = delete;

    explicit Context(
        const CommandLineOptions& options, 
        std::shared_ptr<PyramidizerOperation::IProgressReport> progress_report,
        std::optional<std::tuple<libCZI::CompressionMode, std::shared_ptr<libCZI::ICompressParameters>>> compression_mode_determined_from_source,
        bool background_is_white)
        : command_line_options_(options),
            progress_report_(std::move(progress_report)),
            compression_mode_determined_from_source_(std::move(compression_mode_determined_from_source)),
            background_is_white_(background_is_white)
    {
    }

    IDecimationProcessing* GetDecimationProcessing() override
    {
        return ::GetDecimationProcessing();
    }

    std::shared_ptr<PyramidizerOperation::IProgressReport> GetProgressReporter() override
    {
        return this->progress_report_;
    }

    const CommandLineOptions* GetCommandLineOptions() override
    {
        return &this->command_line_options_;
    }

    std::optional<std::tuple<libCZI::CompressionMode, std::shared_ptr<libCZI::ICompressParameters>>> GetCompressionModeDeterminedFromSource() override
    {
        return this->compression_mode_determined_from_source_;
    }

    libCZI::RgbFloatColor GetBackgroundColor(int channel_number) override
    {
        // note: for the time being, we choose the background color globally for the whole document
        return this->background_is_white_ ? libCZI::RgbFloatColor{ 1.0f, 1.0f, 1.0f } : libCZI::RgbFloatColor{ 0.0f, 0.0f, 0.0f };
    }
};

std::unique_ptr<IContext> IContext::CreateUp(
    const CommandLineOptions& options, 
    const std::shared_ptr<PyramidizerOperation::IProgressReport>& progress_report,
    const std::optional<std::tuple<libCZI::CompressionMode, std::shared_ptr<libCZI::ICompressParameters>>>& compression_mode_determined_from_source,
    bool background_is_white)
{
    return std::make_unique<Context>(options, progress_report, compression_mode_determined_from_source, background_is_white);
}

std::shared_ptr<IContext> IContext::CreateSp(
    const CommandLineOptions& options, 
    const std::shared_ptr<PyramidizerOperation::IProgressReport>& progress_report,
    const std::optional<std::tuple<libCZI::CompressionMode, std::shared_ptr<libCZI::ICompressParameters>>>& compression_mode_determined_from_source,
    bool background_is_white)
{
    return std::make_shared<Context>(options, progress_report, compression_mode_determined_from_source, background_is_white);
}
