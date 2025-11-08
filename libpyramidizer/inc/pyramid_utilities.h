// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>

#include <cstdint>

namespace libpyramidizer
{
    class PyramidUtilities
    {
    public:
        /// Move the edge-points of the specified rectangle so that they are multiples of the specified value,
        /// and give the rectangle which satisfies this condition and is the smallest possible.
        ///
        /// \param  rect        The rectangle to latch to multiples of the specified value.
        /// \param  multiple_of The value the coordinates must be a multiple of.
        ///
        /// \returns    The adjusted rectangle.
        static libCZI::IntRect EnlargeAndLatch(const libCZI::IntRect& rect, std::uint32_t multiple_of);
    };
}
