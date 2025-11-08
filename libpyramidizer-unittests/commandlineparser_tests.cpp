// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include "inc/commandlineoptions.h"

#include <array>

using namespace std;
using namespace libCZI;
using namespace libpyramidizer;

TEST(CommandlineArgsParsingTests, Scenario1)
{
    constexpr int argc = 7;
    std::array<const char*,argc> argv = { "czi-pyramidizer", "-s", "source.czi", "-d", "destination.czi" , "--compressionopts", "zstd1:ExplicitLevel=2;PreProcess=HiLoByteUnpack"};

    CommandLineOptions options;
    const auto result = options.Parse(argc, const_cast<char**>(argv.data()));

    ASSERT_EQ(CommandLineOptions::ParseResult::OK, result);
    ASSERT_EQ(L"source.czi", options.GetSourceCziUri());
    ASSERT_EQ(L"destination.czi", options.GetDestinationCziUri());
    ASSERT_EQ(libCZI::CompressionMode::Zstd1, options.GetCompressionMode());
    auto compression_parameters = options.GetCompressionParameters();
    ASSERT_NE(nullptr, compression_parameters);
    CompressParameter compression_parameter;
    ASSERT_TRUE(compression_parameters->TryGetProperty(libCZI::CompressionParameterKey::ZSTD_RAWCOMPRESSIONLEVEL, &compression_parameter));
    ASSERT_EQ(compression_parameter.GetType(), CompressParameter::Type::Int32);
    ASSERT_EQ(2, compression_parameter.GetInt32());
    ASSERT_TRUE(compression_parameters->TryGetProperty(libCZI::CompressionParameterKey::ZSTD_PREPROCESS_DOLOHIBYTEPACKING, &compression_parameter));
    ASSERT_EQ(compression_parameter.GetType(), CompressParameter::Type::Boolean);
    ASSERT_EQ(true, compression_parameter.GetBoolean());
}

TEST(CommandlineArgsParsingTests, Scenario2)
{
    constexpr int argc = 7;
    std::array<const char*,argc> argv = { "czi-pyramidizer", "-s", "source.czi", "-d", "destination.czi" , "--compressionopts", "uncompressed:"};

    CommandLineOptions options;
    const auto result = options.Parse(argc, const_cast<char**>(argv.data()));

    ASSERT_EQ(CommandLineOptions::ParseResult::OK, result);
    ASSERT_EQ(L"source.czi", options.GetSourceCziUri());
    ASSERT_EQ(L"destination.czi", options.GetDestinationCziUri());
    ASSERT_EQ(libCZI::CompressionMode::UnCompressed, options.GetCompressionMode());
}
