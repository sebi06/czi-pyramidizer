[![REUSE status](https://api.reuse.software/badge/github.com/ZEISS/czi-pyramidizer)](https://api.reuse.software/info/github.com/ZEISS/czi-pyramidizer)
[![CMake Linux-x64](https://github.com/ZEISS/czi-pyramidizer/actions/workflows/cmake_linux_x64.yml/badge.svg?branch=main&event=push)](https://github.com/ZEISS/czi-pyramidizer/actions/workflows/cmake_linux_x64)
[![CMake Windows-x64](https://github.com/ZEISS/czi-pyramidizer/actions/workflows/cmake_windows_x64.yml/badge.svg?branch=main&event=push)](https://github.com/ZEISS/czi-pyramidizer/actions/workflows/cmake_windows_x64)
[![MegaLinter](https://github.com/ZEISS/czi-pyramidizer/actions/workflows/megalinter.yml/badge.svg?branch=main&event=push)](https://github.com/ZEISS/czi-pyramidizer/actions/workflows/megalinter.yml)

# Introduction 

czi-pyramidizer is a stand-alone tool (libCZI-based) for creating a multi-resolution pyramid for a given CZI.

It is expecially useful for adding pyramids in CZI-documents that have been created with [libCZI](https://github.com/ZEISS/libczi), [pylibCZI](https://github.com/ZEISS/pylibczirw) or related tools,
that do not support automatic pyramid creation. The availability of a multi-resolution pyramid is a prerequisite for efficient visualization of large images in [ZEN](https://www.zeiss.com/microscopy/en/products/software/zeiss-zen.html) or other image viewers.

# Features

- Create multi-resolution pyramids for CZI files
- Check whether a CZI file already contains a pyramid

## Guidelines
[Code of Conduct](./CODE_OF_CONDUCT.md)  
[Contributing](./CONTRIBUTING.md)

## Disclaimer
ZEISS, ZEISS.com are registered trademarks of Carl Zeiss AG.
