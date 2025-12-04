# Getting Started

Follow these steps to build and run `czi-pyramidizer`.

## 1. Install Prerequisites

- CMake (>=3.15)
- C++17 toolchain
- OpenCV development packages

For preparing the build environment, consider using [vcpkg](https://github.com/microsoft/vcpkg) to get the OpenCV dependency.  
The commands for vcpkg are:
```
vcpkg install opencv4[core,jpeg,png,quirc,tiff,webp]:x64-windows-static
```

Then, you need to add those flags to the CMake configure command:
```
-DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
```



## 2. Clone Repository

```
git clone https://github.com/ZEISS/czi-pyramidizer.git
cd czi-pyramidizer
```

## 3. Configure & Build

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j
```

Append additional flags as needed (see above for vcpkg).

## 4. Run unit tests (optional)

```
ctest
```


## 5. Next Steps

- Explore `docs/usage/cli.md`

## Troubleshooting

| Problem | Hint |
|---------|------|
| Missing OpenCV headers | Install `libopencv-dev` (Linux) or use vcpkg. |

