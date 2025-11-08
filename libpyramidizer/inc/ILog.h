// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

namespace libpyramidizer
{
    const int LOGLEVEL_CATASTROPHICERROR = 0;   ///< Identifies a catastrophic error (i. e. the program cannot continue).
    const int LOGLEVEL_ERROR = 1;               ///< Identifies a non-recoverable error.
    const int LOGLEVEL_SEVEREWARNING = 2;       ///< Identifies that a severe problem has occured. Proper operation of the module is not ensured.
    const int LOGLEVEL_WARNING = 3;             ///< Identifies that a problem has been identified. It is likely that proper operation can be kept up.
    const int LOGLEVEL_INFORMATION = 4;         ///< Identifies an informational output. It has no impact on the proper operation.
    const int LOGLEVEL_CHATTYINFORMATION = 5;   ///< Identifies an informational output which has no impact on proper operation. Use this for output which may occur with high frequency.

    class ILog
    {
    public:
        /// Query if  the specified logging level is enabled. In the case that constructing the message
        /// to be logged takes a significant amount of resources (i. e. time or memory), this method should
        /// be called before in order to determine whether the output is required at all. This also means
        /// that this method may be called very frequently, so implementors should take care that it executes
        /// reasonably fast.
        ///
        /// \param logLevel The logging level.
        ///
        /// \return True if the specified logging level is enabled, false otherwise.
        virtual bool IsEnabled(int logLevel) = 0;

        /// Output the specified string at the specified logging level.
        /// \remark
        /// The text is assumed to be ASCII - not UTF8 or any other codepage. Use only plain-ASCII.
        /// This might change...
        /// \param level The logging level.
        /// \param szMsg The message to be logged.
        virtual void Log(int level, const char* szMsg) = 0;

        virtual ~ILog() = default;

        // non-copyable and non-moveable
        ILog() = default;
        ILog(const ILog&) = default;             // copy constructor
        ILog& operator=(const ILog&) = default;  // copy assignment
        ILog(ILog&&) = default;                  // move constructor
        ILog& operator=(ILog&&) = default;       // move assignment
    };
} // namespace libpyramidizer
