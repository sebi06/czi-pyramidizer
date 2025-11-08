// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#include "../inc/utilities.h"
#include <libpyramidizer_config.h>

#if LIBPYRAMIDIZER_WIN32_ENVIRONMENT
#include <Windows.h>
#endif
#if LIBPYRAMIDIZER_UNIX_ENVIRONMENT
#include <iconv.h>
#include <vector>
#include <cstring>
#endif

#include <algorithm>
#include <stdexcept>

std::wstring libpyramidizer::Utilities::utf8_to_wstring(const std::string& utf8_string)
{
#if LIBPYRAMIDIZER_WIN32_ENVIRONMENT
    if (utf8_string.empty())
    {
        return {};
    }

    // Calculate the length of the resulting wide string
    int wide_length = MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, nullptr, 0);
    if (wide_length == 0)
    {
        // Handle error
        return {};
    }

    // Allocate space for the wide string
    std::wstring wide_string(wide_length, 0);

    // Convert the UTF-8 string to a wide string
    int result = MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, wide_string.data(), wide_length);
    if (result == 0)
    {
        // Handle error
        return {};
    }

    // Remove the null terminator added by MultiByteToWideChar
    wide_string.resize(wide_length - 1);

    return wide_string;
#endif
#if LIBPYRAMIDIZER_UNIX_ENVIRONMENT
    if (utf8_string.empty()) 
    {
        return std::wstring();
    }

    // Open an iconv conversion descriptor
    iconv_t cd = iconv_open("WCHAR_T", "UTF-8");
    if (cd == (iconv_t)-1)
    {
        throw std::runtime_error("iconv_open failed: " + std::string(strerror(errno)));
    }

    // Prepare input buffer
    const char* inbuf = utf8_string.data();
    size_t inbytesleft = utf8_string.size();

    // Prepare output buffer
    size_t outbuf_size = (inbytesleft + 1) * sizeof(wchar_t);
    std::vector<wchar_t> wbuffer(outbuf_size / sizeof(wchar_t));
    char* outbuf = reinterpret_cast<char*>(wbuffer.data());
    size_t outbytesleft = outbuf_size;

    // Pointers to keep track of buffer positions
    char* inbuf_ptr = const_cast<char*>(inbuf);
    char* outbuf_ptr = outbuf;

    // Perform the conversion
    while (inbytesleft > 0)
    {
        size_t result = iconv(cd, &inbuf_ptr, &inbytesleft, &outbuf_ptr, &outbytesleft);
        if (result == (size_t)-1)
        {
            if (errno == E2BIG)
            {
                // Output buffer is full, resize it
                size_t outbuf_used = outbuf_ptr - outbuf;
                outbuf_size *= 2;
                wbuffer.resize(outbuf_size / sizeof(wchar_t));
                outbuf = reinterpret_cast<char*>(wbuffer.data());
                outbuf_ptr = outbuf + outbuf_used;
                outbytesleft = outbuf_size - outbuf_used;
            }
            else if (errno == EILSEQ)
            {
                iconv_close(cd);
                throw std::runtime_error("Invalid multibyte sequence encountered.");
            }
            else if (errno == EINVAL)
            {
                iconv_close(cd);
                throw std::runtime_error("Incomplete multibyte sequence encountered.");
            }
            else
            {
                iconv_close(cd);
                throw std::runtime_error("iconv conversion error: " + std::string(strerror(errno)));
            }
        }
    }

    // Clean up
    iconv_close(cd);

    // Calculate the number of wchar_t converted
    size_t outbytes_used = outbuf_ptr - reinterpret_cast<char*>(wbuffer.data());
    size_t wchar_count = outbytes_used / sizeof(wchar_t);

    // Create the wstring from the buffer
    std::wstring wstr(wbuffer.data(), wchar_count);

    return wstr;
#endif
}

/*static*/std::string libpyramidizer::Utilities::trim(const std::string& str)
{
    // Find the first non-whitespace character
    const size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) 
    {
        // All characters are whitespace
        return "";
    }

    // Find the last non-whitespace character
    const size_t end = str.find_last_not_of(" \t\n\r\f\v");

    // Extract the substring without leading and trailing whitespaces
    return str.substr(start, end - start + 1);
}

/*static*/std::string libpyramidizer::Utilities::toLower(const std::string& str)
{
    std::string result = str;
    std::transform(
        result.begin(), 
        result.end(), 
        result.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return result;
}
