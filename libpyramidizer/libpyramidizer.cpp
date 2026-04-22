// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "libpyramidizer.h"
#include <libpyramidizer_config.h>
#include "inc/commandlineoptions.h"
#include "inc/do_pyramidize.h"
#include "inc/pyramidizer_operation.h"
#include "inc/CheckForPyramid.h"

#include <libCZI.h>
#include <string>
#include <sstream>

#if LIBPYRAMIDIZER_WIN32_ENVIRONMENT
#include <Windows.h>
#endif

using namespace std;
using namespace libpyramidizer;
using namespace libCZI;

namespace
{
    class ProgressReport : public libpyramidizer::PyramidizerOperation::IProgressReport
    {
        std::shared_ptr<libpyramidizer::IConsoleIo> console_io_;
        std::uint32_t last_time_line_length_;
    public:
        ProgressReport(const std::shared_ptr<libpyramidizer::IConsoleIo>& console_io)
            : console_io_(console_io), last_time_line_length_(0)
        {
            this->console_io_->WriteLineStdOut("");
            this->console_io_->MoveUp(1);
        }

        void ReportProgressPyramidizer(const libCZI::CDimCoordinate& plane_coordinate, std::uint8_t layer, std::uint32_t total_count_of_tiles, std::uint32_t count_of_tiles_done) override
        {
            string coordinate_as_string = Utils::DimCoordinateToString(&plane_coordinate);

            uint32_t line_length = 0;
            this->console_io_->SetColor(ConsoleColor::LIGHT_YELLOW, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut("Pyramidize: "); line_length += 12;
            this->console_io_->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut("plane "); line_length += 6;
            this->console_io_->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut(coordinate_as_string); line_length += coordinate_as_string.size();
            this->console_io_->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut(", layer "); line_length += 8;
            this->console_io_->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
            string text = to_string(static_cast<uint32_t>(layer));
            this->console_io_->WriteStdOut(text); line_length += text.size();
            this->console_io_->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut(", "); line_length += 2;
            this->console_io_->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
            text = to_string(count_of_tiles_done);
            this->console_io_->WriteStdOut(text); line_length += text.size();
            this->console_io_->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut(" of "); line_length += 5;
            this->console_io_->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
            text = to_string(total_count_of_tiles);
            this->console_io_->WriteStdOut(text); line_length += text.size();
            this->console_io_->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
            if (total_count_of_tiles != 1)
            {
                this->console_io_->WriteStdOut(" tiles done."); line_length += 12;
            }
            else
            {
                this->console_io_->WriteStdOut(" tile done."); line_length += 11;
            }

            this->console_io_->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
            this->ClearRemainderAndSetNewLineLength(line_length);
            this->console_io_->WriteStdOut("\r");
        }

        void ReportProgressCopySubBlock(const libCZI::CDimCoordinate& plane_coordinate, std::uint32_t total_count_of_tiles, std::uint32_t count_of_tiles_done) override
        {
            string coordinate_as_string = Utils::DimCoordinateToString(&plane_coordinate);

            uint32_t line_length = 0;
            this->console_io_->SetColor(ConsoleColor::LIGHT_YELLOW, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut("Copy Layer0: "); line_length += 13;
            this->console_io_->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut("plane "); line_length += 6;
            this->console_io_->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut(coordinate_as_string); line_length += coordinate_as_string.size();
            this->console_io_->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut(", "); line_length += 2;
            this->console_io_->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
            string text = to_string(count_of_tiles_done);
            this->console_io_->WriteStdOut(text); line_length += text.size();
            this->console_io_->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut(" of "); line_length += 4;
            this->console_io_->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
            text = to_string(total_count_of_tiles);
            this->console_io_->WriteStdOut(text); line_length += text.size();
            this->console_io_->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
            if (total_count_of_tiles != 1)
            {
                this->console_io_->WriteStdOut(" subblocks done."); line_length += 16;
            }
            else
            {
                this->console_io_->WriteStdOut(" subblock done."); line_length += 15;
            }

            this->console_io_->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
            this->ClearRemainderAndSetNewLineLength(line_length);
            this->console_io_->WriteStdOut("\r");
        }

        void ReportProgressCopyAttachment(std::uint32_t total_count_of_attachments, std::uint32_t count_of_attachments_done) override
        {
            uint32_t line_length = 0;
            this->console_io_->SetColor(ConsoleColor::LIGHT_YELLOW, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut("Copy Attachments: "); line_length += 18;
            this->console_io_->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
            string text = to_string(count_of_attachments_done);
            this->console_io_->WriteStdOut(text); line_length += text.size();
            this->console_io_->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
            this->console_io_->WriteStdOut(" attachments done."); line_length += 18;
            this->console_io_->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
            this->ClearRemainderAndSetNewLineLength(line_length);
            this->console_io_->WriteStdOut("\r");
        }
    private:
        void ClearRemainderAndSetNewLineLength(uint32_t new_line_length)
        {
            if (new_line_length < this->last_time_line_length_)
            {
                string spaces(this->last_time_line_length_ - new_line_length, ' ');
                this->console_io_->WriteStdOut(spaces);
            }

            this->last_time_line_length_ = new_line_length;
        }
    };

    shared_ptr<libCZI::IStream> CreateSourceStream(const CommandLineOptions& command_line_options)
    {
        if (command_line_options.GetSourceCziStreamClass().empty())
        {
            return libCZI::CreateStreamFromFile(command_line_options.GetSourceCziUri().c_str());
        }
        else
        {
            libCZI::StreamsFactory::Initialize();
            libCZI::StreamsFactory::CreateStreamInfo stream_info;
            stream_info.class_name = command_line_options.GetSourceCziStreamClass();
            stream_info.property_bag = command_line_options.GetInputStreamPropertyBag();
            auto stream = libCZI::StreamsFactory::CreateStream(stream_info, command_line_options.GetSourceCziUri());
            if (!stream)
            {
                stringstream string_stream;
                string_stream << "Failed to create stream object of the class \"" << stream_info.class_name << "\".";
                throw std::runtime_error(string_stream.str());
            }

            return stream;
        }
    }
}

int libpyramidizer::libmain(int argc, char* argv[], const std::shared_ptr<IConsoleIo>& console_io)
{
    CommandLineOptions command_line_options(console_io);
    const auto parse_result = command_line_options.Parse(argc, argv);

    switch (parse_result)
    {
        case CommandLineOptions::ParseResult::OK:
            break;
        case CommandLineOptions::ParseResult::Exit:
            return LIBPYRAMIDIZER_RETCODE_SUCCESS;
        case CommandLineOptions::ParseResult::Error:
            return LIBPYRAMIDIZER_RETCODE_INVALID_ARGUMENTS;
    }

#if LIBPYRAMIDIZER_WIN32_ENVIRONMENT
    CoInitialize(NULL);
#endif

    // on Windows, let's use the WIC decoder (for de-compressing JPGXR)
    libCZI::ISite* site = libCZI::GetDefaultSiteObject(libCZI::SiteObjectType::WithWICDecoder);
    if (site != nullptr)
    {
        libCZI::SetSiteObject(site);
    }

    int exit_code = LIBPYRAMIDIZER_RETCODE_SUCCESS;
    try
    {
        switch (command_line_options.GetCommand())
        {
            case Command::Pyramidize:
            {
                PyramidizerOperation::PyramidizerOperationOptions options;
                options.command_line_options = &command_line_options;
                options.console_io = console_io;
                options.source_stream = CreateSourceStream(command_line_options);
                options.get_destination_stream_functor = [&command_line_options]()
                    {
                        return libCZI::CreateOutputStreamForFile(
                            command_line_options.GetDestinationCziUri().c_str(),
                            command_line_options.GetOverwriteDestination());
                    };
                options.progress_report = make_shared<ProgressReport>(console_io);

                PyramidizerOperation pyramidizer_operation(options);
                const auto return_code = pyramidizer_operation.Execute();
                if (return_code == PyramidizerOperation::Result::NoOperationNecessary)
                {
                    exit_code = LIBPYRAMIDIZER_RETCODE_NOACTIONBECAUSENOPYRAMIDNEEDED;
                }

                console_io->WriteLineStdOut("");
            }
            break;
            case Command::CheckOnly:
            {
                const auto source_stream = CreateSourceStream(command_line_options);
                const bool needs_pyramid = CheckForPyramid::NeedsPyramid(source_stream, command_line_options);
                if (needs_pyramid)
                {
                    console_io->WriteLineStdOut("For the specified document a pyramid should be created.");
                    exit_code = LIBPYRAMIDIZER_RETCODE_PYRAMIDISNEEDEDED;
                }
                else
                {
                    console_io->WriteLineStdOut("The specified document does not need a pyramid or already contains it.");
                    exit_code = LIBPYRAMIDIZER_RETCODE_SUCCESS;
                }
            }
        }
    }
    catch (const exception& e)
    {
        console_io->WriteLineStdOut("");
        console_io->WriteLineStdOut("");
        stringstream ss;
        ss << "An unhandled exception occurred : " << e.what();
        console_io->WriteLineStdErr(ss.str());
        exit_code = LIBPYRAMIDIZER_RETCODE_GENERAL_ERROR;
    }

#if LIBPYRAMIDIZER_WIN32_ENVIRONMENT
    CoUninitialize();
#endif

    return exit_code;
}
