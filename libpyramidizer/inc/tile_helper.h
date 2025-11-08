// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>

#include <vector>
#include <iterator>
#include <type_traits>

namespace libpyramidizer
{
    /// This utility does the following: given a set of tiles, it finds the AABB of intersections of the tiles with 
    /// the given rectangle. It is useful to find the minimal rectangle which contains the ROI and is overlapping with the tiles.
    /// Use it for find the minimal pyramid-sub-block which contains the ROI.
    class TileHelper
    {
    public:
        template <typename Range>
        void Initialize(const Range& rectangles)
        {
            static_assert(std::is_same_v<typename std::iterator_traits<decltype(std::begin(rectangles))>::value_type, libCZI::IntRect>,
                "Range must contain libCZI::IntRect");

            tiles_.assign(std::begin(rectangles), std::end(rectangles));
        }

        void InitializeFromFunctor(const std::function<bool(libCZI::IntRect&)>& enumeration_function);

        libCZI::IntRect GetMinimalRectForRoi(const libCZI::IntRect& rect) const;
    private:
        std::vector <libCZI::IntRect> tiles_;
    };
} // namespace libpyramidizer
