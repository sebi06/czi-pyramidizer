// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "../inc/UpperPyramidLayersPyramidizer.h"
#include "../inc/tile_helper.h"
#include "../inc/pyramid_utilities.h"
#include "../inc/tiling_enumerator.h"
#include "../inc/IContext.h"
#include "../inc/IDecimationProcessing.h"

#include <libCZI.h>

#include "imaging/compose.h"

using namespace std;
using namespace libCZI;
using namespace libpyramidizer;

void UpperPyramidLayersPyramidizer::Execute()
{
    const auto pixel_type = this->options_.tiles.front()->GetBitmapData()->GetPixelType();

    TileHelper tile_helper = this->ConstructTileHelper();

    const uint32_t minification_factor = 1 << this->options_.pyramid_layer;
    TilingEnumerator tiling_enumerator(
        PyramidUtilities::EnlargeAndLatch(this->options_.region, minification_factor),
        minification_factor * this->options_.context->GetCommandLineOptions()->GetPyramidTileWidth(),
        minification_factor * this->options_.context->GetCommandLineOptions()->GetPyramidTileHeight());


    if (this->options_.progress_report)
    {
        this->options_.progress_report(tiling_enumerator.GetCountOfTiles(), 0);
    }

    uint32_t tiles_processed = 1;
    for (const auto tile_rectangle : tiling_enumerator)
    {
        const auto minimal_rect = tile_helper.GetMinimalRectForRoi(tile_rectangle);
        if (minimal_rect.IsNonEmpty())
        {
            const auto minimal_tile_latched = PyramidUtilities::EnlargeAndLatch(minimal_rect, minification_factor);

            auto tiles_within_rect = this->GetTilesIntersecting(minimal_tile_latched);

            // if the tile is too small, we discard it (and we get a nullptr here)
            if (auto pyramid_tile = this->CreatePyramidTile(pixel_type, minimal_tile_latched, tiles_within_rect))
            {
                this->options_.output_tile_functor(pyramid_tile, minimal_tile_latched);
            }
        }

        if (this->options_.progress_report)
        {
            this->options_.progress_report(tiling_enumerator.GetCountOfTiles(), tiles_processed);
        }

        ++tiles_processed;
    }
}

TileHelper UpperPyramidLayersPyramidizer::ConstructTileHelper() const
{
    TileHelper tile_helper;
    auto tiles_iterator = this->options_.tiles.begin();
    auto tiles_end = this->options_.tiles.end();
    tile_helper.InitializeFromFunctor(
        [&tiles_iterator, tiles_end](IntRect& rect)->bool
        {
            if (tiles_iterator != tiles_end)
            {
                rect = (*tiles_iterator)->GetRect();
                ++tiles_iterator;
                return true;
            }

            return false;
        });

    return tile_helper;
}

std::vector<size_t> UpperPyramidLayersPyramidizer::GetTilesIntersecting(const libCZI::IntRect& rect) const
{
    std::vector<size_t> result;
    for (size_t i = 0; i < this->options_.tiles.size(); ++i)
    {
        if (this->options_.tiles[i]->GetRect().Intersect(rect).IsNonEmpty())
        {
            result.push_back(i);
        }
    }

    return result;
}

std::shared_ptr<libCZI::IBitmapData> UpperPyramidLayersPyramidizer::CreatePyramidTile(libCZI::PixelType pixel_type, const libCZI::IntRect& roi, const std::vector<size_t>& tiles_within_rect)
{
    const auto roi_scaled = this->ScaleByPyramidLayerMinusOne(roi);
    if (roi_scaled.w < 2 || roi_scaled.h < 2)
    {
        return nullptr;
    }

    // TODO(JBL): we don't have the channel-number here at hand, so we just use 0 (which is fine since for now we don't care about the channel number)
    auto iterator = tiles_within_rect.begin();
    const auto end_iterator = tiles_within_rect.end();
    const auto composition = ComposeBitmaps::Compose(
        roi_scaled,
        pixel_type,
        this->options_.context->GetBackgroundColor(0),
        [&, this](std::shared_ptr<IBitmapData>& bitmap, int& x, int& y)->bool
        {
            if (iterator == end_iterator)
            {
                return false;
            }

            const size_t index = *iterator++;

            const auto& tile = this->options_.tiles[index];
            bitmap = tile->GetBitmapData();

            const auto rect = tile->GetRect();
            x = this->ScaleByPyramidLayerMinusOne(rect.x - roi.x);
            y = this->ScaleByPyramidLayerMinusOne(rect.y - roi.y);

            return true;
        });

    // OK, so now create the pyramid-tile
    return this->options_.context->GetDecimationProcessing()->Decimate(composition);
}

int UpperPyramidLayersPyramidizer::ScaleByPyramidLayer(int x) const
{
    return x >> this->options_.pyramid_layer;
}

int UpperPyramidLayersPyramidizer::ScaleByPyramidLayerMinusOne(int x) const
{
    return x >> (this->options_.pyramid_layer - 1);
}

libCZI::IntRect UpperPyramidLayersPyramidizer::ScaleByPyramidLayer(const libCZI::IntRect& rect) const
{
    return libCZI::IntRect{
        ScaleByPyramidLayer(rect.x),
        ScaleByPyramidLayer(rect.y),
        ScaleByPyramidLayer(rect.w),
        ScaleByPyramidLayer(rect.h)
    };
}

libCZI::IntRect UpperPyramidLayersPyramidizer::ScaleByPyramidLayerMinusOne(const libCZI::IntRect& rect) const
{
    return libCZI::IntRect{
        ScaleByPyramidLayerMinusOne(rect.x),
        ScaleByPyramidLayerMinusOne(rect.y),
        ScaleByPyramidLayerMinusOne(rect.w),
        ScaleByPyramidLayerMinusOne(rect.h)
    };
}
