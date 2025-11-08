// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "../inc/pyramidizer_operation.h"
#include "../inc/IContext.h"
#include "../IConsoleIo.h"
#include "inc/do_pyramidize.h"
#include "inc/CheckForPyramid.h"
#include <libCZI.h>
#include <optional>
#include <iomanip>
#include <gsl/gsl>

using namespace libCZI;
using namespace std;
using namespace libpyramidizer;

PyramidizerOperation::PyramidizerOperation(PyramidizerOperationOptions options)
    : options_(std::move(options))
{
    if (!this->options_.command_line_options)
    {
        throw invalid_argument("command_line_options must not be nullptr");
    }

    if (!this->options_.source_stream)
    {
        throw invalid_argument("source_stream must not be nullptr");
    }

    if (!this->options_.get_destination_stream_functor)
    {
        throw invalid_argument("get_destination_stream_functor must not be nullptr");
    }
}

PyramidizerOperation::Result PyramidizerOperation::Execute()
{
    const auto reader = libCZI::CreateCZIReader();
    reader->Open(this->options_.source_stream);

    const bool continue_operation = this->CheckForNecessityOfPyramid(reader);
    if (!continue_operation)
    {
        return Result::NoOperationNecessary;
    }

    bool white_background_color = false;
    const auto background_color_option = this->GetCommandLineOptions().GetBackgroundColorOption();
    switch (background_color_option.mode)
    {
        case BackgroundColorOption::Mode::Auto:
            white_background_color = this->DetermineWhetherBackgroundShouldBeWhite(reader);
            break;
        case BackgroundColorOption::Mode::UseFixed:
            white_background_color = background_color_option.white_or_black;
            break;
    }

    auto compression_mode_determined_from_file = this->TryToDetermineCompressionMode(reader);

    const auto context = IContext::CreateSp(
                                    this->GetCommandLineOptions(),
                                    this->options_.progress_report,
                                    compression_mode_determined_from_file,
                                    white_background_color);

    if (this->options_.command_line_options->GetLogVerbosityLevel() >= LOGLEVEL_INFORMATION && this->options_.console_io)
    {
        this->PrintOperationalParameters(context);
        this->PrintSourceDocumentInfo(reader);
    }

    const auto writer = libCZI::CreateCZIWriter();

    auto destination_stream = this->options_.get_destination_stream_functor();

    // GUID_NULL here means that a new Guid is created
    const auto czi_writer_info = make_shared<libCZI::CCziWriterInfo>(libCZI::GUID{ 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0} });

    // TODO(JBL): it might be desirable to make reservations for the
    //            subblock-directory-/attachments-directory-/metadata-segment
    //            (so that the end up at the beginning of the file instead of at
    //            the end), but this shouldn't make a difference in terms of
    //            proper operation
    writer->Create(destination_stream, czi_writer_info);

    DoPyramidize do_pyramidize;
    DoPyramidize::PyramidizeOptions options;
    options.reader = reader;
    options.writer = writer;
    options.context = context;

    do_pyramidize.Initialize(options);
    const auto pyramidizing_result = do_pyramidize.Execute();

    // now, copy the metadata
    const auto metadata_segment = reader->ReadMetadataSegment();
    if (metadata_segment)
    {
        this->WriteMetadata(pyramidizing_result, metadata_segment, writer);
    }

    // finally, copy all attachments
    this->CopyAttachments(reader, writer);

    writer->Close();
    return Result::OperationSuccessful;
}

void PyramidizerOperation::WriteMetadata(const DoPyramidize::PyramidizeResult& pyramidizing_result, const std::shared_ptr<libCZI::IMetadataSegment>& metadata_segment, const std::shared_ptr<libCZI::ICziWriter>& writer)
{
    const auto source_metadata = metadata_segment->CreateMetaFromMetadataSegment();
    const auto metadata_builder = CreateMetadataBuilderFromXml(source_metadata->GetXml());

    const auto root_node = metadata_builder->GetRootNode();

    for (const auto& [scene_index, pyramid_region_info] : pyramidizing_result.pyramid_region_info)
    {
        int scene_index_consolidated = scene_index;
        if (scene_index == numeric_limits<int>::min())
        {
            // for the case "no S-index", we will get numeric_limits<int>::min() here, and we then use an S-index of zero.
            scene_index_consolidated = 0;
        }

        ostringstream pyramid_info_node_path;
        pyramid_info_node_path << "Metadata/Information/Image/Dimensions/S/Scenes/Scene[Index=" << scene_index_consolidated << "]/PyramidInfo";
        const auto pyramid_info_scene0_node = root_node->GetOrCreateChildNode(pyramid_info_node_path.str());
        pyramid_info_scene0_node->GetOrCreateChildNode("IsSane")->SetValueBool(true);
        pyramid_info_scene0_node->GetOrCreateChildNode("IsMinimalCoverage")->SetValueBool(true);
        pyramid_info_scene0_node->GetOrCreateChildNode("MinificationFactor")->SetValueUI32(2);
        pyramid_info_scene0_node->GetOrCreateChildNode("MinificationMethod")->SetValue("Gaussian"); // TODO(JBL): so far, only this method is implemented
        pyramid_info_scene0_node->GetOrCreateChildNode("PyramidLayersCount")->SetValueUI32(pyramid_region_info.number_of_pyramid_layers);
    }

    WriteMetadataInfo metadata_info;
    metadata_info.Clear();
    const string source_metadata_xml = metadata_builder->GetXml();
    metadata_info.szMetadata = source_metadata_xml.c_str();
    metadata_info.szMetadataSize = source_metadata_xml.size();
    writer->SyncWriteMetadata(metadata_info);
}

void PyramidizerOperation::CopyAttachments(const std::shared_ptr<libCZI::ICZIReader>& reader, const std::shared_ptr<libCZI::ICziWriter>& writer)
{
    // For the time being, just do a simple copy

    // report progress
    constexpr uint32_t total_count_of_attachments = numeric_limits<uint32_t>::max();  // TODO(JBL): we don't know the total count of tiles, maybe we should change this
    if (auto progress_report = this->options_.progress_report)
    {
        progress_report->ReportProgressCopyAttachment(
            total_count_of_attachments,
            0);
    }

    uint32_t number_of_attachments_done = 1;
    reader->EnumerateAttachments(
        [&](int index, const libCZI::AttachmentInfo&) -> bool
        {
            const auto attachment = reader->ReadAttachment(index);
            this->WriteAttachment(writer, attachment);

            if (auto progress_report = this->options_.progress_report)
            {
                progress_report->ReportProgressCopyAttachment(
                    total_count_of_attachments,
                    number_of_attachments_done);
            }

            ++number_of_attachments_done;

            return true;
        });
}

void PyramidizerOperation::WriteAttachment(const std::shared_ptr<libCZI::ICziWriter>& writer, const std::shared_ptr<libCZI::IAttachment>& attachment)
{
    const auto guid = attachment->GetAttachmentInfo().contentGuid;
    size_t size_raw_xml_data{ 0 };
    const auto metadata_raw_xml_data = attachment->GetRawData(&size_raw_xml_data);
    AddAttachmentInfo add_attachment_info{ guid, {}, {}, metadata_raw_xml_data.get(), gsl::narrow<uint32_t>(size_raw_xml_data) };
    add_attachment_info.SetContentFileType(attachment->GetAttachmentInfo().contentFileType);  // NOLINT
    add_attachment_info.SetName(attachment->GetAttachmentInfo().name.c_str());
    writer->SyncAddAttachment(add_attachment_info);
}

void PyramidizerOperation::PrintSourceDocumentInfo(const std::shared_ptr<libCZI::ICZIReader>& reader)
{
    /*
    We want to print out something like this:

    SubBlock-Statistics
    -------------------

    SubBlock-Count: 2681
    Bounding-Box (All): X=-186083 Y=24840 W=73923 H=53212
    Bounding-Box (Layer0): X=-186083 Y=24840 W=73731 H=53212
    M-Index: min=0 max=1053
    Bounds:
        C -> start=0 size=1
        S -> start=0 size=2
        B -> start=0 size=1

    We use those "styles", something like this:

    <Header1>
    SubBlock-Statistics
    -------------------
    </Header1>

    <Label1>Bounding-Box (All):</Label1> <Label2>X=</Label2><Value>-186083</Value> <Label2>Y=</Label2><Value>24840</Value> ...

    <Header2>Bounds:</Header2>
    */

    const auto statistics = reader->GetStatistics();

    const auto& console_io = this->options_.console_io;

    this->SetOutputColorForHeader1();
    console_io->WriteLineStdOut("SubBlock-Statistics");
    console_io->WriteLineStdOut("-------------------");
    console_io->WriteLineStdOut("");

    this->SetOutputColorForLabel1();
    console_io->WriteStdOut("SubBlock-Count: ");
    this->SetOutputColorForValue();
    console_io->WriteLineStdOut(to_string(statistics.subBlockCount));
    this->SetOutputColorForLabel1();
    console_io->WriteStdOut("Bounding-Box (All)   : ");
    PrintIntRect(console_io, statistics.boundingBox);
    console_io->WriteLineStdOut("");
    this->SetOutputColorForLabel1();
    console_io->WriteStdOut("Bounding-Box (Layer0): ");
    PrintIntRect(console_io, statistics.boundingBoxLayer0Only);
    console_io->WriteLineStdOut("");

    this->SetOutputColorForLabel1();
    console_io->WriteStdOut("M-Index: ");
    if (statistics.IsMIndexValid())
    {
        this->SetOutputColorForLabel2();
        console_io->WriteStdOut("min=");
        this->SetOutputColorForValue();
        console_io->WriteStdOut(to_string(statistics.minMindex));
        this->SetOutputColorForLabel2();
        console_io->WriteStdOut(" max=");
        this->SetOutputColorForValue();
        console_io->WriteStdOut(to_string(statistics.maxMindex));
    }
    else
    {
        this->SetOutputColorForValue();
        console_io->WriteStdOut("not available");
    }

    console_io->WriteLineStdOut("");

    this->SetOutputColorForHeader2();
    console_io->WriteLineStdOut("Bounds:");
    statistics.dimBounds.EnumValidDimensions(
                            [&](libCZI::DimensionIndex dim, int start, int size)->bool
                            {
                                this->SetOutputColorForValue();
                                string text{ Utils::DimensionToChar(dim) };
                                console_io->WriteStdOut("  ");
                                console_io->WriteStdOut(text);
                                this->SetOutputColorForLabel2();
                                console_io->WriteStdOut(" -> start=");
                                this->SetOutputColorForValue();
                                console_io->WriteStdOut(to_string(start));
                                this->SetOutputColorForLabel2();
                                console_io->WriteStdOut(" size=");
                                this->SetOutputColorForValue();
                                console_io->WriteStdOut(to_string(size));
                                console_io->WriteLineStdOut("");
                                return true;
                            });

    if (!statistics.sceneBoundingBoxes.empty())
    {
        this->SetOutputColorForHeader2();
        console_io->WriteLineStdOut("Scene-BoundingBoxes:");
        for (const auto& sceneBoundingBox : statistics.sceneBoundingBoxes)
        {
            this->SetOutputColorForLabel1();
            console_io->WriteStdOut("  Scene ");
            this->SetOutputColorForValue();
            console_io->WriteStdOut(to_string(sceneBoundingBox.first));
            this->SetOutputColorForLabel1();
            console_io->WriteLineStdOut(":");
            this->SetOutputColorForLabel1();
            console_io->WriteStdOut("    All   : ");
            PrintIntRect(console_io, sceneBoundingBox.second.boundingBox);
            console_io->WriteLineStdOut("");
            this->SetOutputColorForLabel1();
            console_io->WriteStdOut("    Layer0: ");
            PrintIntRect(console_io, sceneBoundingBox.second.boundingBoxLayer0);
            console_io->WriteLineStdOut("");
        }
    }

    this->ResetOutputColor();
    console_io->WriteLineStdOut("");
}

void PyramidizerOperation::PrintIntRect(const std::shared_ptr<IConsoleIo>& console_io, const libCZI::IntRect& rect)
{
    this->SetOutputColorForLabel2();
    console_io->WriteStdOut("X=");
    this->SetOutputColorForValue();
    console_io->WriteStdOut(to_string(rect.x));
    this->SetOutputColorForLabel2();
    console_io->WriteStdOut(" Y=");
    this->SetOutputColorForValue();
    console_io->WriteStdOut(to_string(rect.y));
    this->SetOutputColorForLabel2();
    console_io->WriteStdOut(" W=");
    this->SetOutputColorForValue();
    console_io->WriteStdOut(to_string(rect.w));
    this->SetOutputColorForLabel2();
    console_io->WriteStdOut(" H=");
    this->SetOutputColorForValue();
    console_io->WriteStdOut(to_string(rect.h));
}

void PyramidizerOperation::SetOutputColorForHeader1()
{
    if (this->options_.console_io->IsStdOutATerminal())
    {
        this->options_.console_io->SetColor(ConsoleColor::LIGHT_YELLOW, ConsoleColor::DEFAULT);
    }
}

void PyramidizerOperation::SetOutputColorForHeader2()
{
    if (this->options_.console_io->IsStdOutATerminal())
    {
        this->options_.console_io->SetColor(ConsoleColor::LIGHT_CYAN, ConsoleColor::DEFAULT);
    }
}

void PyramidizerOperation::SetOutputColorForValue()
{
    if (this->options_.console_io->IsStdOutATerminal())
    {
        this->options_.console_io->SetColor(ConsoleColor::DARK_GREEN, ConsoleColor::DEFAULT);
    }
}

void PyramidizerOperation::SetOutputColorForLabel1()
{
    if (this->options_.console_io->IsStdOutATerminal())
    {
        this->options_.console_io->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
    }
}

void PyramidizerOperation::SetOutputColorForLabel2()
{
    if (this->options_.console_io->IsStdOutATerminal())
    {
        this->options_.console_io->SetColor(ConsoleColor::DARK_WHITE, ConsoleColor::DEFAULT);
    }
}

void PyramidizerOperation::ResetOutputColor()
{
    if (this->options_.console_io->IsStdOutATerminal())
    {
        this->options_.console_io->SetColor(ConsoleColor::DEFAULT, ConsoleColor::DEFAULT);
    }
}

std::optional<std::tuple<libCZI::CompressionMode, std::shared_ptr<libCZI::ICompressParameters>>>
PyramidizerOperation::TryToDetermineCompressionMode(const std::shared_ptr<libCZI::ICZIReader>& reader)
{
    // If the user has specified a compression mode, we use that one
    if (this->GetCommandLineOptions().GetCompressionMode() != CompressionMode::Invalid)
    {
        return std::nullopt;
    }

    // If the user has not specified a compression mode, we use the one from the source file
    // 
    // - we enumerate all subblocks, and check the first non-pyramid subblock we encounter
    // - we then use the compression mode of that subblock
    optional<CompressionMode> compression_mode;
    reader->EnumSubset(
        nullptr,
        nullptr,
        true,   /* onlyLayer0 */
        [&compression_mode](int index, const libCZI::SubBlockInfo& subBlockInfo) -> bool
            {
                compression_mode = subBlockInfo.GetCompressionMode();
                return false;
            });

    if (!compression_mode.has_value())
    {
        return std::nullopt;
    }

    switch (compression_mode.value())
    {
        case CompressionMode::JpgXr:
        {
            auto compress_parameters = make_shared<CompressParametersOnMap>();
            compress_parameters->map[static_cast<int>(CompressionParameterKey::JXRLIB_QUALITY)] = CompressParameter(static_cast<uint32_t>(950));
            return make_tuple(CompressionMode::JpgXr, compress_parameters);
        }
        case CompressionMode::Zstd0:
        {
            auto compress_parameters = make_shared<CompressParametersOnMap>();
            compress_parameters->map[static_cast<int>(CompressionParameterKey::ZSTD_RAWCOMPRESSIONLEVEL)] = CompressParameter(static_cast<int32_t>(0));
            return make_tuple(CompressionMode::Zstd0, compress_parameters);
        }
        case CompressionMode::Zstd1:
        {
            auto compress_parameters = make_shared<CompressParametersOnMap>();
            compress_parameters->map[static_cast<int>(CompressionParameterKey::ZSTD_RAWCOMPRESSIONLEVEL)] = CompressParameter(static_cast<int32_t>(0));
            compress_parameters->map[static_cast<int>(CompressionParameterKey::ZSTD_PREPROCESS_DOLOHIBYTEPACKING)] = CompressParameter(true);
            return make_tuple(CompressionMode::Zstd1, compress_parameters);
        }
        case CompressionMode::UnCompressed:
            return make_tuple(CompressionMode::UnCompressed, nullptr);
        default:
            return std::nullopt;
    }
}

void PyramidizerOperation::PrintOperationalParameters(const std::shared_ptr<libpyramidizer::IContext>& context)
{
    const auto& console_io = this->options_.console_io;

    this->SetOutputColorForHeader1();
    console_io->WriteLineStdOut("Operational Parameters");
    console_io->WriteLineStdOut("----------------------");
    console_io->WriteLineStdOut("");

    this->SetOutputColorForLabel1();
    console_io->WriteStdOut("Source-CZI-URI: ");
    this->SetOutputColorForValue();
    console_io->WriteLineStdOut(context->GetCommandLineOptions()->GetSourceCziUri());

    this->SetOutputColorForLabel1();
    console_io->WriteStdOut("Destination-CZI-URI: ");
    this->SetOutputColorForValue();
    console_io->WriteLineStdOut(context->GetCommandLineOptions()->GetDestinationCziUri());

    this->SetOutputColorForLabel1();
    console_io->WriteStdOut("Pyramid-Tile-Size: ");
    this->SetOutputColorForValue();
    console_io->WriteLineStdOut(to_string(context->GetCommandLineOptions()->GetPyramidTileWidth()));

    this->SetOutputColorForLabel1();
    console_io->WriteStdOut("Max-Top-Level-Pyramid-Size: ");
    this->SetOutputColorForValue();
    console_io->WriteLineStdOut(to_string(context->GetCommandLineOptions()->TopLevelPyramidTileMaxWidth()));

    this->SetOutputColorForLabel1();
    console_io->WriteStdOut("Compression-Mode: ");
    this->SetOutputColorForValue();
    libCZI::CompressionMode compression_mode = context->GetCommandLineOptions()->GetCompressionMode();
    if (compression_mode == libCZI::CompressionMode::Invalid)
    {
        const auto compression_mode_determined_from_source = context->GetCompressionModeDeterminedFromSource();
        if (compression_mode_determined_from_source.has_value())
        {
            console_io->WriteStdOut(Utils::CompressionModeToInformalString(get<0>(compression_mode_determined_from_source.value())));
            console_io->WriteLineStdOut(" (from source file)");
        }
        else
        {
            console_io->WriteLineStdOut("not specified (using 'uncompressed')");
        }
    }
    else
    {
        console_io->WriteStdOut(Utils::CompressionModeToInformalString(compression_mode));
        console_io->WriteLineStdOut(" (from arguments)");
    }

    this->SetOutputColorForLabel1();
    console_io->WriteStdOut("Background Color: ");
    this->SetOutputColorForValue();
    ostringstream string_stream;
    string_stream << "(" << std::defaultfloat << std::setprecision(2)
        << context->GetBackgroundColor(0).r << ","
        << context->GetBackgroundColor(0).g << ","
        << context->GetBackgroundColor(0).b << ")";
    console_io->WriteLineStdOut(string_stream.str());

    this->ResetOutputColor();
    console_io->WriteLineStdOut("");
}

bool PyramidizerOperation::DetermineWhetherBackgroundShouldBeWhite(const std::shared_ptr<libCZI::ICZIReader>& reader)
{
    const auto metadata_segment = reader->ReadMetadataSegment();
    if (!metadata_segment)
    {
        return false;
    }

    const auto metadata = metadata_segment->CreateMetaFromMetadataSegment();
    if (!metadata)
    {
        return false;
    }

    // first, we check the "DisplaySetting"-node - if all channels are "Transmitted" or "Reflected", we assume a white background
    if (const auto display_settings_channels_node = metadata->GetChildNodeReadonly("ImageDocument/Metadata/DisplaySetting/Channels"))
    {
        uint32_t total_channel_node_count = 0;
        uint32_t number_of_white_background_channels = 0;
        display_settings_channels_node->EnumChildren(
            [&total_channel_node_count, &number_of_white_background_channels](std::shared_ptr<IXmlNodeRead> child_node)->bool
            {
                if (child_node->Name() == L"Channel")
                {
                    ++total_channel_node_count;
                    if (const auto illumination_node = child_node->GetChildNodeReadonly("IlluminationType"))
                    {
                        wstring value;
                        if (illumination_node->TryGetValue(&value))
                        {
                            if (value == L"Transmitted" || value == L"Reflected")
                            {
                                ++number_of_white_background_channels;
                            }
                        }

                    }
                }

                return true;
            });

        if (total_channel_node_count > 0 && total_channel_node_count == number_of_white_background_channels)
        {
            return true;
        }
    }

    // if above method was inconclusive, then check the Dimensions/Channels node - in this case we can use the "typed access layer"
    if (const auto multi_dimensional_document_info = metadata->GetDocumentInfo())
    {
        if (const auto dimension_channel_info = multi_dimensional_document_info->GetDimensionChannelsInfo())
        {
            int number_of_white_background_channels = 0;
            for (int i = 0; i < dimension_channel_info->GetChannelCount(); ++i)
            {
                if (const auto dimension_channel = dimension_channel_info->GetChannel(i))
                {
                    DimensionChannelIlluminationType illumination_type;
                    if (dimension_channel->TryGetIlluminationType(&illumination_type))
                    {
                        if (illumination_type == DimensionChannelIlluminationType::Transmitted || illumination_type == DimensionChannelIlluminationType::Oblique)
                        {
                            ++number_of_white_background_channels;
                        }
                    }
                }
            }

            if (number_of_white_background_channels == dimension_channel_info->GetChannelCount())
            {
                return true;
            }
        }
    }

    return false;
}

bool PyramidizerOperation::CheckForNecessityOfPyramid(const std::shared_ptr<libCZI::ICZIReader>& reader)
{
    if (this->options_.command_line_options->GetModeOfOperation() == ModeOfOperation::OnlyOperateIfPyramidIsNeeded)
    {
        const bool needs_pyramid = CheckForPyramid::NeedsPyramid(reader, *this->options_.command_line_options);
        if (!needs_pyramid && this->options_.console_io)
        {
            this->options_.console_io->WriteLineStdOut("The specified document does not need a pyramid or already contains it.");
        }

        return needs_pyramid;
    }

    return true;
}
