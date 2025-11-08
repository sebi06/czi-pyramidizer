// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IConsoleIo.h"

#include <memory>

namespace libpyramidizer
{
    /// Main entry point for the library, providing "terminal based operation". The
    /// arguments provided here are the command-line parameters, and the console-io
    /// is an abstraction used for terminal-output.
    /// The return value indicates error or success of the operation.
    /// 
    /// | value | meaning                                               |
    /// |-------|-------------------------------------------------------|
    /// | 0     | The operation completed successfully. If checking whether a pyramid is needed, this indicates that there is no need to generate a pyramid - it is either not needed or already present. |
    /// | 1     | Generic failure code - some kind of error occurred, which is not further characterized by the exit code.   |
    /// | 10    | If checking if a pyramid is needed, this exit code indicates that a pyramid is needed.|
    /// | 11    | It was determined that no pyramid needs to be created. |
    /// | 99    | There was an error parsing the command-line arguments.|
    ///
    /// \param          argc       The argc.
    /// \param [in,out] argv       If non-null, the argv.
    /// \param          console_io The console i/o.
    ///
    /// \returns An integer code indicating failure or success of the operation.
    int libmain(int argc, char* argv[], const std::shared_ptr<IConsoleIo>& console_io);

    constexpr int LIBPYRAMIDIZER_RETCODE_SUCCESS                        = 0;
    constexpr int LIBPYRAMIDIZER_RETCODE_GENERAL_ERROR                  = 1;
    constexpr int LIBPYRAMIDIZER_RETCODE_PYRAMIDISNEEDEDED              = 10;
    constexpr int LIBPYRAMIDIZER_RETCODE_NOACTIONBECAUSENOPYRAMIDNEEDED = 11;
    constexpr int LIBPYRAMIDIZER_RETCODE_INVALID_ARGUMENTS              = 99;
}
