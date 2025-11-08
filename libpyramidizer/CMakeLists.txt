# SPDX-FileCopyrightText: 2024 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

set(LIB_PYRAMIDIZER_SRCFILES
 "src/commandlineoptions.cpp"
 "inc/commandlineoptions.h"
 "libpyramidizer.h" 
 "libpyramidizer.cpp" 
 "inc/do_pyramidize.h" 
 "inc/tile_helper.h" 
 "src/tile_helper.cpp"
 "src/do_pyramidize.cpp" 
 "inc/plane_enumerator.h"
 "inc/tiling_enumerator.h"
 "src/imaging/decimate_opencv.h"
 "src/imaging/decimate_opencv.cpp"
 "inc/IDecimationProcessing.h" 
 "src/imaging/bitmapdata.h" 
 "src/imaging/bitmapdata.cpp"
 "src/DecimationProcessing.cpp"
 "inc/IContext.h" 
 "src/Context.cpp" 
 "inc/UpperPyramidLayersPyramidizer.h" 
 "src/UpperPyramidLayersPyramidizer.cpp" 
 "src/imaging/compose.h" 
 "src/imaging/compose.cpp" 
 "src/imaging/opencvutils.h" 
 "src/imaging/opencvutils.cpp"
 "src/pyramidizer_operation.cpp"
 "inc/pyramidizer_operation.h" 
 "inc/utilities.h" 
 "src/utilities.cpp"
 "IConsoleIo.h" 
 "inc/ILog.h"
 "inc/pyramid_utilities.h"
 "src/pyramid_utilities.cpp"
 "inc/CheckForPyramid.h"
 "src/CheckForPyramid.cpp")

add_library(libpyramidizer ${LIB_PYRAMIDIZER_SRCFILES})

target_compile_definitions(libpyramidizer PRIVATE _LIBCZISTATICLIB)
target_link_libraries(libpyramidizer PRIVATE libCZIStatic CLI11::CLI11 GSL ${OpenCV_LIBS})
target_include_directories(libpyramidizer PRIVATE ${LIBCZI_INCLUDE_DIR} ${RAPIDJSON_INCLUDE_DIRS})
target_compile_definitions(libpyramidizer PRIVATE RAPIDJSON_HAS_STDSTRING=1)
target_include_directories(libpyramidizer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(libpyramidizer PROPERTIES CXX_STANDARD 17)

# get the URL of the upstream repository
string(CONCAT libpyramidizer_CompilerIdentification ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION} )

execute_process(
  COMMAND git remote get-url origin
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE libpyramidizer_REPOSITORYREMOTEURL
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (NOT DEFINED libpyramidizer_REPOSITORYREMOTEURL OR "${libpyramidizer_REPOSITORYREMOTEURL}" STREQUAL "")
  set(libpyramidizer_REPOSITORYREMOTEURL "unknown")
endif()

execute_process(
  COMMAND git name-rev --name-only HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE libpyramidizer_REPOSITORYBRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (NOT DEFINED libpyramidizer_REPOSITORYBRANCH OR "${libpyramidizer_REPOSITORYBRANCH}" STREQUAL "")
  set(libpyramidizer_REPOSITORYBRANCH "unknown")
endif()

execute_process(
  COMMAND  git log -1 --format=%H
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE libpyramidizer_REPOSITORYHASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (NOT DEFINED libpyramidizer_REPOSITORYHASH OR "${libpyramidizer_REPOSITORYHASH}" STREQUAL "")
  set(libpyramidizer_REPOSITORYHASH "unknown")
endif()

if (NOT CZIPYRAMIDIZER_HAVE_WIN32_API)
  target_link_libraries(libpyramidizer PRIVATE Iconv::Iconv)
endif()

if (CZIPYRAMIDIZER_HAVE_WIN32_API)
	set(libpyramidizer_UNIX_ENVIRONMENT 0)
	set(libpyramidizer_WIN32_ENVIRONMENT 1)
else()
  set(libpyramidizer_UNIX_ENVIRONMENT 1)
	set(libpyramidizer_WIN32_ENVIRONMENT 0)
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/libpyramidizer_config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/libpyramidizer_config.h")
target_include_directories(libpyramidizer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})