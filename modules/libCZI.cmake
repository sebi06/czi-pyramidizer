# SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

include(FetchContent)

# Set necessary build options for libCZI
set(LIBCZI_BUILD_CZICMD OFF CACHE BOOL "" FORCE)
set(LIBCZI_BUILD_DYNLIB OFF CACHE BOOL "" FORCE)
set(LIBCZI_BUILD_UNITTESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  libCZI
  GIT_REPOSITORY https://github.com/ZEISS/libczi
  #GIT_TAG        origin/main
  GIT_TAG        da3d7dcef3e1d9fb0f0b59325b0a4c77e081523c # main as of 11/7/2025
)

# Fetch the content and make it available
FetchContent_MakeAvailable(libCZI)