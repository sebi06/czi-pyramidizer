// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>
#include <memory>
#include <map>

namespace libpyramidizer
{
    class IContext;

    /// This class is orchestrating the operation.
    class DoPyramidize
    {
    public:
        /// The options which are to be provided for running the operation.
        struct PyramidizeOptions
        {
            /// The reader object - representing the source document.
            std::shared_ptr<libCZI::ICZIReader> reader;

            /// The writer object - representing the destination document.
            std::shared_ptr<libCZI::ICziWriter> writer;

            ///< The context object - representing the "external" context for the operation.
            std::shared_ptr<IContext> context;  
        };

        /// Information about the operation on one "pyramid region". A "pyramid region" is either a scene
        /// (in case the S-index is used with the source document) or the whole document in case that no
        /// S-index is used.
        struct PyramidRegionInfo
        {
            std::uint32_t number_of_pyramid_layers{0};  ///< Number of pyramid layers.
        };

        /// Encapsulates the "result of a pyramidize operation". Here we gather all information that 
        /// might be relevant for the caller.
        struct PyramidizeResult
        {
            /// Information describing the pyramid region. The key is the scene-index in case the source document 
            /// has an S-index, and numerical_limits<int>::min() if there was no S-index used.
            std::map<int, PyramidRegionInfo> pyramid_region_info;   
        };
    private:
        PyramidizeOptions options_;

    public:
        DoPyramidize();

        /// Destructor - note that the destructor MUST be defined in the implementation file, we cannot use the default destructor here,
        /// because the unique_ptr to the implementation class is a forward declaration.
        ~DoPyramidize();    

        // Copy Constructor 
        DoPyramidize(const DoPyramidize& other) = delete;

        // Copy Assignment Operator 
        DoPyramidize& operator=(const DoPyramidize& other) = delete;

        void Initialize(const PyramidizeOptions& options);

        PyramidizeResult Execute();
    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;    // pointer to the implementation (PIMPL-idiom)
        void ThrowIfNotInitialized() const;
    };
} // namespace libpyramidizer
