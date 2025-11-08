// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <string>
#include <cstdint>
#include "../IConsoleIo.h"

#include <libCZI.h>

namespace libpyramidizer
{
    /// This enum defines the possible "commands", i.e. what the application should do.
    enum class Command
    {
        Invalid,    ///< The 'invalid' option.
        CheckOnly,  ///< Only check whether the source documents contains a pyramid.
        Pyramidize, ///< Create a pyramid.
    };

    /// This enum defines "modes of operation", i.e. how the application should behave.
    enum class ModeOfOperation
    {
        Invalid,                        ///< The 'invalid' option.
        OnlyOperateIfPyramidIsNeeded,   ///< Only check whether the source documents contains a pyramid.
        AlwaysGeneratePyramid,          ///< Always generate a pyramid - or attempt to create a pyramid, irrespective of whether a pyramid is already present.
    };

    enum class CompressionMethodPolicy
    {
        /// Automatic selection of the compression method. The implementation is free to choose the most appropriate method.
        Auto,

        /// Use the compression method as specified by the user.
        AsSpecified
    };

    /// This structure defines the background color option.
    struct BackgroundColorOption
    {
        /// Values that represent how the background color is to be determined.
        enum class Mode : std::uint8_t
        {
            Auto,       ///< The background color is to be determined automatically.
            UseFixed,   ///< The color indicated in this struct is to be used.
        };

        Mode mode;  ///< The background color determination mode.

        bool white_or_black;    ///< True means "white background" and false means "black background". This property is only valid if mode is "Fixed".
    };

    class CommandLineOptions
    {
    private:
        static constexpr std::uint32_t kDefaultPyramidTileSize = 1024;
        static constexpr std::uint32_t kDefaultMaxTopLevelPyramidSize = 1024;
        std::shared_ptr<IConsoleIo> console_io_;
        std::wstring source_czi_uri_;
        std::string source_czi_stream_class_;
        std::map<int, libCZI::StreamsFactory::Property> property_bag_for_stream_class_;
        std::wstring destination_czi_uri_;
        std::uint32_t pyramid_tile_size_{ kDefaultPyramidTileSize };
        std::uint32_t max_top_level_pyramid_size_{ kDefaultMaxTopLevelPyramidSize };
        libCZI::CompressionMode compression_mode_{ libCZI::CompressionMode::Invalid };
        std::shared_ptr<libCZI::ICompressParameters> compression_parameters_;
        BackgroundColorOption background_color_option_{ BackgroundColorOption::Mode::Auto, false };
        int log_verbosity_level_{ 0 };
        bool overwrite_destination_{ false };
        std::uint32_t threshold_when_pyramid_is_required_{ 4096 };
        Command command_{ Command::Invalid };
        ModeOfOperation mode_of_operation_{ ModeOfOperation::Invalid };
    public:
        /// Values that represent the result of the "Parse"-operation.
        enum class ParseResult : std::uint8_t
        {
            OK,     ///< An enum constant representing the result "arguments successfully parsed, operation can start".
            Exit,   ///< An enum constant representing the result "operation complete, the program should now be terminated, e.g. the synopsis was printed".
            Error   ///< An enum constant representing the result "there was an error parsing the command line arguments, program should terminate".
        };

        CommandLineOptions() : CommandLineOptions(nullptr) {}
        explicit CommandLineOptions(const std::shared_ptr<IConsoleIo>& console_io);
        ~CommandLineOptions() = default;

        /// Parse the command line arguments. The arguments are expected to have
        /// UTF8-encoding.
        ///
        /// \param          argc    The number of arguments.
        /// \param [in]     argv    Pointer to an array with the null-terminated, UTF8-encoded arguments.
        ///
        /// \returns    True if it succeeds, false if it fails.
        [[nodiscard]] ParseResult Parse(int argc, char** argv);

        [[nodiscard]] const std::wstring& GetSourceCziUri() const { return this->source_czi_uri_; }
        [[nodiscard]] const std::string& GetSourceCziStreamClass() const { return this->source_czi_stream_class_; }
        [[nodiscard]] const std::map<int, libCZI::StreamsFactory::Property>& GetInputStreamPropertyBag() const { return this->property_bag_for_stream_class_; }
        [[nodiscard]] const std::wstring& GetDestinationCziUri() const { return this->destination_czi_uri_; }

        [[nodiscard]] std::uint32_t GetPyramidTileWidth() const { return this->pyramid_tile_size_; }
        [[nodiscard]] std::uint32_t GetPyramidTileHeight() const { return this->pyramid_tile_size_; }

        /// The max width of the top-level pyramid tile. Pyramid layers are added until the top-level pyramid tile is smaller than this width.
        ///
        /// \returns    The max width of the top-level pyramid tile.
        [[nodiscard]] std::uint32_t TopLevelPyramidTileMaxWidth() const { return this->max_top_level_pyramid_size_; }

        /// The max height of the top-level pyramid tile. Pyramid layers are added until the top-level pyramid tile is smaller than this height.
        ///
        /// \returns    The max height of the top-level pyramid tile.
        [[nodiscard]] std::uint32_t TopLevelPyramidTileMaxHeight() const { return this->max_top_level_pyramid_size_; }

        /// Gets compression mode given on the command line. This will give CompressionMode::Invalid if no compression mode was given.
        /// In this case, the application should use some default compression mode.
        ///
        /// \returns    The compression mode.
        [[nodiscard]] libCZI::CompressionMode GetCompressionMode() const { return this->compression_mode_; }

        [[nodiscard]] std::shared_ptr<libCZI::ICompressParameters> GetCompressionParameters() const { return this->compression_parameters_; }

        [[nodiscard]] int GetLogVerbosityLevel() const { return this->log_verbosity_level_; }

        [[nodiscard]] bool GetOverwriteDestination() const { return this->overwrite_destination_; }

        [[nodiscard]] BackgroundColorOption GetBackgroundColorOption() const { return this->background_color_option_; }

        [[nodiscard]] Command GetCommand() const { return this->command_; }

        [[nodiscard]] std::uint32_t GetThresholdWhenPyramidIsRequired() const { return this->threshold_when_pyramid_is_required_; }

        [[nodiscard]] ModeOfOperation GetModeOfOperation() const { return this->mode_of_operation_; }
    private:
        friend struct CompressionOptionsValidator;
        friend struct BackgroundColorOptionValidator;

        static bool TryParseCompressionOptions(const std::string& s, libCZI::Utils::CompressionOption* compression_option);
        static bool TryParseInputStreamCreationPropertyBag(const std::string& s, std::map<int, libCZI::StreamsFactory::Property>* property_bag);
        static bool TryParseBackgroundColorOption(const std::string& s, BackgroundColorOption* background_color_option);

        static void ThrowIfFalse(bool b, const std::string& argument_switch, const std::string& argument);

        static std::string GetAppDescription();
        static std::string GetFooterText();

        void PrintHelpBuildInfo();
        void PrintHelpStreamsObjects();

        void DoConsole(const std::function<void(IConsoleIo*)>& console_io_function) const
        {
            if (this->console_io_)
            {
                console_io_function(this->console_io_.get());
            }
        }
    };
} // namespace libpyramidizer
