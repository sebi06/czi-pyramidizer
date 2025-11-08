# SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest"
    GIT_TAG        v1.17.0
)

# For Windows : Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(libpyramidizer-unittests
 "tile_helper_test.cpp"
 "plane_enumerator_tests.cpp" 
 "tiling_enumerator_tests.cpp" 
 "imaging_tests.cpp" 
 "commandlineparser_tests.cpp" 
 "pyramidizer_operation_test.cpp" 
 "libCZIUtilities.h" 
 "libCZIUtilities.cpp" 
 "pyramid_utils_test.cpp" 
 "check_for_pyramid_tests.cpp")

set_target_properties(libpyramidizer-unittests PROPERTIES CXX_STANDARD 17)
target_link_libraries(libpyramidizer-unittests PRIVATE libpyramidizer GTest::gtest_main)
target_include_directories(libpyramidizer-unittests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../libpyramidizer" "${LIBCZI_INCLUDE_DIR}")

set_target_properties(libpyramidizer-unittests PROPERTIES CXX_STANDARD 17)

target_compile_definitions(libpyramidizer-unittests PRIVATE _LIBCZISTATICLIB)

include(GoogleTest)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" AND CMAKE_VERSION VERSION_LESS "3.30.5")
    # no idea why, but 'gtest_discover_tests' doesn't work on macOS with older CMake versions
    gtest_add_tests(TARGET libpyramidizer-unittests)
else()
    gtest_discover_tests(libpyramidizer-unittests)
endif()
