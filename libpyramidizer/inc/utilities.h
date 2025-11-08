// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace libpyramidizer
{
    class Utilities
    {
    public:
        static std::wstring utf8_to_wstring(const std::string& utf8_string);

        /// Function to trim leading and trailing whitespaces.
        ///
        /// \param  string  The string to be trimmed.
        ///
        /// \returns    The trimmed string.
        static std::string trim(const std::string& string);

        /// Convert a string to lowercase.
        ///
        /// \param  str The string to be converted to lowercase.
        ///
        /// \returns    The input string, converted to all lowercase.
        static std::string toLower(const std::string& str);
    };

} // namespace libpyramidizer
