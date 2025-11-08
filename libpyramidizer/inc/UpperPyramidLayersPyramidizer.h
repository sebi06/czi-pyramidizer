// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>
#include <memory>
#include <vector>

#include "tile_helper.h"

namespace libpyramidizer
{
    class IContext;

    class UpperPyramidLayersPyramidizer
    {
    public:
        class ITile
        {
        public:
            [[nodiscard]] virtual libCZI::IntRect GetRect() const = 0;
            [[nodiscard]] virtual std::shared_ptr<libCZI::IBitmapData> GetBitmapData() const = 0;
            virtual ~ITile() = default;
        };

        struct PyramidizerOptions
        {
            std::shared_ptr<IContext> context;
            std::vector<std::shared_ptr<ITile>> tiles;
            libCZI::IntRect region;
            std::uint8_t pyramid_layer;
            std::function<void(const std::shared_ptr<libCZI::IBitmapData>&, const libCZI::IntRect&)> output_tile_functor;
            std::function<void(std::uint32_t, std::uint32_t)> progress_report;
        };
    private:
        const PyramidizerOptions& options_;
    public:
        UpperPyramidLayersPyramidizer(const PyramidizerOptions& options) : options_(options) {}

        // Copy Constructor 
        UpperPyramidLayersPyramidizer(const UpperPyramidLayersPyramidizer& other) = delete;

        // Copy Assignment Operator 
        UpperPyramidLayersPyramidizer& operator=(const UpperPyramidLayersPyramidizer& other) = delete;

        void Execute();

    private:
        TileHelper ConstructTileHelper() const;
        std::vector<size_t> GetTilesIntersecting(const libCZI::IntRect& rect) const;
        std::shared_ptr<libCZI::IBitmapData> CreatePyramidTile(libCZI::PixelType pixel_type, const libCZI::IntRect& roi, const std::vector<size_t>& tiles_within_rect);
        int ScaleByPyramidLayer(int x) const;
        int ScaleByPyramidLayerMinusOne(int x) const;
        libCZI::IntRect ScaleByPyramidLayer(const libCZI::IntRect& rect) const;
        libCZI::IntRect ScaleByPyramidLayerMinusOne(const libCZI::IntRect& rect) const;
    };
} // namespace libpyramidizer
