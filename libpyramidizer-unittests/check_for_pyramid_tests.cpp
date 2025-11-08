// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "libCZIUtilities.h"
#include "inc/CheckForPyramid.h"

#include <array>
#include <tuple>
#include <type_traits>

using namespace std;
using namespace libCZI;
using namespace libpyramidizer;

namespace
{
    tuple<shared_ptr<void>, size_t> CreateEmptyTestCzi()
    {
        auto writer = CreateCZIWriter();
        auto outStream = make_shared<CMemOutputStream>(0);

        auto spWriterInfo = make_shared<CCziWriterInfo >();
        writer->Create(outStream, spWriterInfo);

        writer->Close();

        return make_tuple(outStream->GetCopy(nullptr), outStream->GetDataSize());
    }

    shared_ptr<ICziReaderWriter> CreateSingleSubblockCzi(int width, int height)
    {
        auto empty_czi_as_blob = CreateEmptyTestCzi();
        auto reader_writer = libCZI::CreateCZIReaderWriter();
        auto mem_input_output_stream = make_shared<CMemInputOutputStream>(
            get<0>(empty_czi_as_blob).get(),
            get<1>(empty_czi_as_blob),
            8 * 1024 * 1024);   // instruct to grow the buffer in units of 8MB - this speeds up the operation significantly

        reader_writer->Create(mem_input_output_stream);

        auto bitmap = CreateGray8BitmapAndFill(8192, 8192, 0x1);

        libCZI::AddSubBlockInfoStridedBitmap addSbBlkInfo;
        addSbBlkInfo.Clear();
        addSbBlkInfo.coordinate.Set(libCZI::DimensionIndex::C, 0);
        addSbBlkInfo.mIndexValid = false;
        addSbBlkInfo.x = 0;
        addSbBlkInfo.y = 0;
        addSbBlkInfo.logicalWidth = static_cast<int>(bitmap->GetWidth());
        addSbBlkInfo.logicalHeight = static_cast<int>(bitmap->GetHeight());
        addSbBlkInfo.physicalWidth = static_cast<int>(bitmap->GetWidth());
        addSbBlkInfo.physicalHeight = static_cast<int>(bitmap->GetHeight());
        addSbBlkInfo.PixelType = bitmap->GetPixelType();
        {
            const libCZI::ScopedBitmapLockerSP lock_info_bitmap{ bitmap };
            addSbBlkInfo.ptrBitmap = lock_info_bitmap.ptrDataRoi;
            addSbBlkInfo.strideBitmap = lock_info_bitmap.stride;
            reader_writer->SyncAddSubBlock(addSbBlkInfo);
        }

        return reader_writer;
    }

    struct SceneInfo
    {
        int x;
        int y;
        int width;
        int height;
    };

    template<typename Iterator,
        typename = std::enable_if_t< // restrict (with SFINAE) to iterators that have value_type 'SceneInfo'
            std::is_same_v<
                typename std::iterator_traits<Iterator>::value_type,
                SceneInfo
            >>>
    shared_ptr<ICziReaderWriter> CreateMultiSceneCzi(Iterator begin, Iterator end)
    {
        auto empty_czi_as_blob = CreateEmptyTestCzi();
        auto reader_writer = libCZI::CreateCZIReaderWriter();
        auto mem_input_output_stream = make_shared<CMemInputOutputStream>(
            get<0>(empty_czi_as_blob).get(),
            get<1>(empty_czi_as_blob),
            8 * 1024 * 1024);   // instruct to grow the buffer in units of 8MB - this speeds up the operation significantly

        reader_writer->Create(mem_input_output_stream);

        int i = 0;
        for (auto it = begin; it != end; ++it)
        {
            const SceneInfo& scene_info = *it;

            auto bitmap = CreateGray8BitmapAndFill(scene_info.width, scene_info.height, static_cast<uint8_t>(i + 1));

            libCZI::AddSubBlockInfoStridedBitmap addSbBlkInfo;
            addSbBlkInfo.Clear();
            addSbBlkInfo.coordinate.Set(libCZI::DimensionIndex::C, 0);
            addSbBlkInfo.coordinate.Set(libCZI::DimensionIndex::S, i);
            addSbBlkInfo.mIndexValid = false;
            addSbBlkInfo.x = scene_info.x;
            addSbBlkInfo.y = scene_info.y;
            addSbBlkInfo.logicalWidth = static_cast<int>(bitmap->GetWidth());
            addSbBlkInfo.logicalHeight = static_cast<int>(bitmap->GetHeight());
            addSbBlkInfo.physicalWidth = static_cast<int>(bitmap->GetWidth());
            addSbBlkInfo.physicalHeight = static_cast<int>(bitmap->GetHeight());
            addSbBlkInfo.PixelType = bitmap->GetPixelType();
            {
                const libCZI::ScopedBitmapLockerSP lock_info_bitmap{ bitmap };
                addSbBlkInfo.ptrBitmap = lock_info_bitmap.ptrDataRoi;
                addSbBlkInfo.strideBitmap = lock_info_bitmap.stride;
                reader_writer->SyncAddSubBlock(addSbBlkInfo);
            }

            ++i;
        }

        return reader_writer;
    }

    /// This will create a multi-scene document with pyramid. We create a multi-scene document, as specified by the
    /// iterator, and for each scene we create a pyramid until the top-level pyramid-tile is less than or equal to the
    /// specified max_extent.
    ///
    /// \tparam Iterator    The iterator (we expect type 'SceneInfo').
    /// \param  begin       Iterator to the beginning.
    /// \param  end         Iterator to one after the end.
    /// \param  max_extent  The maximum extent, i.e. add pyramid-tiles until they are smaller than this extent.
    ///
    /// \returns    The reader-writer object representing the created document.
    template<typename Iterator,
        typename = std::enable_if_t< // restrict (with SFINAE) to iterators that have value_type 'SceneInfo'
                    std::is_same_v<
                        typename std::iterator_traits<Iterator>::value_type,
                        SceneInfo
                    >>>
    shared_ptr<ICziReaderWriter> CreateMultiSceneCziWithPyramid(Iterator begin, Iterator end, int max_extent)
    {
        auto empty_czi_as_blob = CreateEmptyTestCzi();
        auto reader_writer = libCZI::CreateCZIReaderWriter();
        auto mem_input_output_stream = make_shared<CMemInputOutputStream>(
            get<0>(empty_czi_as_blob).get(),
            get<1>(empty_czi_as_blob),
            8 * 1024 * 1024);   // instruct to grow the buffer in units of 8MB - this speeds up the operation significantly

        reader_writer->Create(mem_input_output_stream);

        int i = 0;
        for (auto it = begin; it != end; ++it)
        {
            const SceneInfo& scene_info = *it;

            auto bitmap = CreateGray8BitmapAndFill(scene_info.width, scene_info.height, static_cast<uint8_t>(i + 1));

            for (int pyramid_layer = 0;; ++pyramid_layer)
            {
                int minification_factor = 1 << pyramid_layer;
                int extent = max(scene_info.width / minification_factor, scene_info.height / minification_factor);
                if (pyramid_layer > 0 && extent <= max_extent)
                {
                    break;
                }

                libCZI::AddSubBlockInfoStridedBitmap addSbBlkInfo;
                addSbBlkInfo.Clear();
                addSbBlkInfo.coordinate.Set(libCZI::DimensionIndex::C, 0);
                addSbBlkInfo.coordinate.Set(libCZI::DimensionIndex::S, i);
                addSbBlkInfo.mIndexValid = false;
                addSbBlkInfo.x = scene_info.x;
                addSbBlkInfo.y = scene_info.y;
                addSbBlkInfo.logicalWidth = static_cast<int>(bitmap->GetWidth());
                addSbBlkInfo.logicalHeight = static_cast<int>(bitmap->GetHeight());
                addSbBlkInfo.physicalWidth = static_cast<int>(bitmap->GetWidth() / minification_factor);
                addSbBlkInfo.physicalHeight = static_cast<int>(bitmap->GetHeight() / minification_factor);
                addSbBlkInfo.PixelType = bitmap->GetPixelType();
                {
                    const libCZI::ScopedBitmapLockerSP lock_info_bitmap{ bitmap };
                    addSbBlkInfo.ptrBitmap = lock_info_bitmap.ptrDataRoi;
                    addSbBlkInfo.strideBitmap = lock_info_bitmap.stride;
                    reader_writer->SyncAddSubBlock(addSbBlkInfo);
                }
            }

            ++i;
        }

        return reader_writer;
    }
}

TEST(CheckForPyramidTests, SingleSubBlockNoScenesAboveThresholdExpectPyramidGenerationIsNecessary)
{
    const auto reader_writer = CreateSingleSubblockCzi(8192, 8192);

    CommandLineOptions command_line_options;
    constexpr int argc = 7;
    constexpr array<const char*, argc> argv =
    {
            "czi-pyramidizer.exe",
            "-s",
            "source.czi",
            "-d",
            "destination.czi",
            "--threshold-for-pyramid",
            "500"
    };

    const auto result = command_line_options.Parse(argc, const_cast<char**>(argv.data()));
    ASSERT_EQ(result, CommandLineOptions::ParseResult::OK);

    bool needs_pyramid = CheckForPyramid::NeedsPyramid(reader_writer, command_line_options);
    ASSERT_TRUE(needs_pyramid);
}

TEST(CheckForPyramidTests, SingleSubBlockNoScenesBelowThresholdExpectNoPyramidGenerationNecessary)
{
    const auto readerWriter = CreateSingleSubblockCzi(8192, 8192);

    CommandLineOptions command_line_options;
    constexpr int argc = 7;
    constexpr array<const char*, argc> argv =
    {
            "czi-pyramidizer.exe",
            "-s",
            "source.czi",
            "-d",
            "destination.czi",
            "--threshold-for-pyramid",
            "8193"
    };

    const auto result = command_line_options.Parse(argc, const_cast<char**>(argv.data()));
    ASSERT_EQ(result, CommandLineOptions::ParseResult::OK);

    bool needs_pyramid = CheckForPyramid::NeedsPyramid(readerWriter, command_line_options);
    ASSERT_FALSE(needs_pyramid);
}

TEST(CheckForPyramidTests, ThreeScenesAllScenesBelowThresholdExpectNoPyramidGenerationNecessary)
{
    constexpr array<SceneInfo, 3> scenes_info =
    {
        SceneInfo{0, 0, 1024, 1024},
        SceneInfo{10000, 10000, 1024, 1024},
        SceneInfo{20000, 20100, 2049, 2049}
    };
    const auto reader_writer = CreateMultiSceneCzi(scenes_info.begin(), scenes_info.end());

    CommandLineOptions command_line_options;
    constexpr int argc = 7;
    std::array<const char*, argc> argv =
    {
        "czi-pyramidizer.exe",
        "-s",
        "source.czi",
        "-d",
        "destination.czi",
        "--threshold-for-pyramid",
        "8192"
    };

    const auto result = command_line_options.Parse(argc, const_cast<char**>(argv.data()));
    ASSERT_EQ(result, CommandLineOptions::ParseResult::OK);

    bool needs_pyramid = CheckForPyramid::NeedsPyramid(reader_writer, command_line_options);
    ASSERT_FALSE(needs_pyramid);
}

TEST(CheckForPyramidTests, ThreeScenesOneSceneAboveThresholdAndAlreadyContainsPyramidExpectNoPyramidGenerationNecessary)
{
    constexpr array<SceneInfo, 3> scenes_info =
    {
        SceneInfo{0, 0, 1024, 1024},
        SceneInfo{10000, 10000, 1024, 1024},
        SceneInfo{20000, 20100, 2049, 2049}
    };
    const auto reader_writer = CreateMultiSceneCziWithPyramid(scenes_info.begin(), scenes_info.end(), 1000);

    CommandLineOptions command_line_options;
    constexpr int argc = 7;
    std::array<const char*, argc> argv =
    {
        "czi-pyramidizer.exe",
        "-s",
        "source.czi",
        "-d",
        "destination.czi",
        "--threshold-for-pyramid",
        "2000"
    };

    const auto result = command_line_options.Parse(argc, const_cast<char**>(argv.data()));
    ASSERT_EQ(result, CommandLineOptions::ParseResult::OK);

    bool needs_pyramid = CheckForPyramid::NeedsPyramid(reader_writer, command_line_options);
    ASSERT_FALSE(needs_pyramid);
}
