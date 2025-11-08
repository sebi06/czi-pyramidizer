// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "../inc/pyramid_utilities.h"

using namespace libCZI;
using namespace libpyramidizer;

/*static*/libCZI::IntRect PyramidUtilities::EnlargeAndLatch(const libCZI::IntRect& rect, std::uint32_t multiple_of)
{
    // TODO(JBL): validate the arguments
    const int x1_latched = (rect.x / multiple_of) * multiple_of;
    const int y1_latched = (rect.y / multiple_of) * multiple_of;

    const int x2_latched = ((rect.x + rect.w + multiple_of - 1) / multiple_of) * multiple_of;
    const int y2_latched = ((rect.y + rect.h + multiple_of - 1) / multiple_of) * multiple_of;

    return IntRect{ x1_latched, y1_latched, x2_latched - x1_latched, y2_latched - y1_latched };
}
