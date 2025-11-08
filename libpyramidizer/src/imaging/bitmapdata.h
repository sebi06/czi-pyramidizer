// SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <libCZI.h>
#include <memory>
#include <atomic>
#include <exception>

namespace libpyramidizer
{
    class CBitmapData : public libCZI::IBitmapData
    {
    private:
        void* ptrData;
        libCZI::PixelType pixeltype;
        std::uint32_t width;
        std::uint32_t height;
        std::uint32_t stride;
        std::atomic<int> lockCnt = ATOMIC_VAR_INIT(0);
    public:
        CBitmapData(libCZI::PixelType pixeltype, std::uint32_t width, std::uint32_t height) :
            CBitmapData(pixeltype, width, height, nullptr, 0)
        {
        }

        CBitmapData(libCZI::PixelType pixeltype, std::uint32_t width, std::uint32_t height, const void* data_to_copy, std::uint32_t data_to_copy_stride) :
            pixeltype(pixeltype), width(width), height(height)
        {
            int bytesPerPel;
            switch (pixeltype)
            {
            case libCZI::PixelType::Bgr24:bytesPerPel = 3; break;
            case libCZI::PixelType::Bgr48:bytesPerPel = 6; break;
            case libCZI::PixelType::Gray8:bytesPerPel = 1; break;
            case libCZI::PixelType::Gray16:bytesPerPel = 2; break;
            case libCZI::PixelType::Gray32Float:bytesPerPel = 4; break;
            case libCZI::PixelType::Gray32:bytesPerPel = 4; break;
            default: throw std::runtime_error("unsupported pixeltype");
            }

            if (pixeltype == libCZI::PixelType::Bgr24)
            {
                this->stride = ((width * bytesPerPel + 3) / 4) * 4;
            }
            else
            {
                this->stride = width * bytesPerPel;
            }

            size_t s = static_cast<size_t>(this->stride) * height;
            this->ptrData = std::malloc(s);

            if (data_to_copy != nullptr)
            {
                size_t line_length = static_cast<size_t>(bytesPerPel) * width;
                for (std::uint32_t y = 0; y < height; ++y)
                {
                    const void* source = static_cast<const std::uint8_t*>(data_to_copy) + static_cast<size_t>(y) * data_to_copy_stride;
                    void* dest = static_cast<std::uint8_t*>(this->ptrData) + static_cast<size_t>(y) * this->stride;
                    memcpy(dest, source, line_length);
                }
            }
        }

        ~CBitmapData() override
        {
            std::free(this->ptrData);
        }

        libCZI::PixelType GetPixelType() const override
        {
            return this->pixeltype;
        }

        libCZI::IntSize	GetSize() const override
        {
            return libCZI::IntSize{ this->width, this->height };
        }

        libCZI::BitmapLockInfo Lock() override
        {
            libCZI::BitmapLockInfo bitmapLockInfo;
            bitmapLockInfo.ptrData = this->ptrData;
            bitmapLockInfo.ptrDataRoi = this->ptrData;
            bitmapLockInfo.stride = this->stride;
            bitmapLockInfo.size = static_cast<uint64_t>(this->stride) * this->height;

            std::atomic_fetch_add(&this->lockCnt, 1);

            return bitmapLockInfo;
        }

        void Unlock() override
        {
            std::atomic_fetch_sub(&this->lockCnt, 1);
        }

        int GetLockCount() const override
        {
            return std::atomic_load(&this->lockCnt);
        }

        static std::shared_ptr<libCZI::IBitmapData> CreateBitmapData(libCZI::PixelType pixel_type, std::uint32_t width, std::uint32_t height);
    };
} // namespace libpyramidizer
