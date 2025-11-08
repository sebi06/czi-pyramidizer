// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>
#include <cstdint>

namespace libpyramidizer
{
    class TilingEnumerator
    {
    public:
        TilingEnumerator(libCZI::IntRect roi, std::uint32_t tileWidth, std::uint32_t tileHeight)
            : roi_(roi),
            tile_width_(tileWidth),
            tile_height_(tileHeight)
        {
            // TODO(JBL): Add some validation here.
        }

        /// Nested class for the iterator.
        class Iterator final
        {
        private:
            const TilingEnumerator& tiling_enumerator_;
            std::uint32_t tile_no_;
        protected:
            /// We restrict the constructor to the TilingEnumerator-class only (by making it protected).
            ///
            /// \param  tiling_enumerator The tiling enumerator object (and - we assume its lifetime to be greater than ours).
            /// \param  tile_no The tiling enumerator object (and - we assume its lifetime to be greater than ours).
            explicit Iterator(const TilingEnumerator& tiling_enumerator, std::uint32_t tile_no) :
                tiling_enumerator_(tiling_enumerator),
                tile_no_(tile_no)
            {
            }

            friend class TilingEnumerator;
        public:
            Iterator() = delete;

            /// Prefix increment operator.
            ///
            /// \returns The result of the operation.
            Iterator& operator++()
            {
                if (this->tile_no_ < this->tiling_enumerator_.GetNumberOfTiles())
                {
                    ++this->tile_no_;
                }

                return *this;
            }

            // Dereference operator
            libCZI::IntRect operator*() const
            {
                return this->tiling_enumerator_.GetRoi(this->tile_no_);
            }

            /// Comparison operator.
            ///
            /// \param  other The other object to compare to.
            ///
            /// \returns True if the parameters are not considered equivalent.
            bool operator!=(const Iterator& other) const
            {
                return this->tile_no_ != other.tile_no_;
            }
        };

        /// Begin iterator.
        ///
        /// \returns An Iterator.
        Iterator begin() const
        {
            return Iterator(*this, 0);
        }

        /// End iterator (one past the last).
        ///
        /// \returns An Iterator pointing to "one after the last element".
        Iterator end() const
        {
            return Iterator(*this, this->GetNumberOfTiles());
        }

        std::uint32_t GetCountOfTiles() const
        {
            return this->GetNumberOfTiles();
        }

    private:
        libCZI::IntRect roi_;
        std::uint32_t tile_width_;
        std::uint32_t tile_height_;

        std::uint32_t GetNumberOfTiles() const
        {
            return ((this->roi_.w + this->tile_width_ - 1) / this->tile_width_) * ((this->roi_.h + this->tile_height_ - 1) / this->tile_height_);
        }

        libCZI::IntRect GetRoi(std::uint32_t tile_no) const
        {
            const std::uint32_t tiles_per_row = (this->roi_.w + this->tile_width_ - 1) / this->tile_width_;
            const std::uint32_t row = tile_no / tiles_per_row;
            const std::uint32_t col = tile_no % tiles_per_row;

            const std::int32_t x = this->roi_.x + (int)(col * this->tile_width_);
            const std::int32_t y = this->roi_.y + (int)(row * this->tile_height_);
            const std::int32_t w = std::min((int)(this->tile_width_), this->roi_.w - (int)(col * this->tile_width_));
            const std::int32_t h = std::min((int)(this->tile_height_), this->roi_.h - (int)(row * this->tile_height_));

            return libCZI::IntRect{ x, y, w, h };
        }
    };
} // namespace libpyramidizer
