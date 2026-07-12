cmake_minimum_required(VERSION 3.16)

project(isoiff)

include_directories(../libisomediafile/src)

# nlohmann/json for reading the shared 'heif_cfen' interchange metadata (already used by t35_tool).
include(FetchContent)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
  UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(json)

include_directories(
  ../HM/source/Lib
  ../HM/source/Lib/TLibCommon
  ../HM/source/Lib/TLibDecoder
)

include_directories(
  # Linux
  $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/linux>

  # Windows
  $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/w32>

  # Mac
  $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/macosx>
)

add_library(
  libisoiff

  src/Logger.c
  src/isoiff.c
  src/HEVCDecoderConfigRecord.c
  src/HEVCItemData.c
  src/HEVCConfigurationAtom.c
  src/ImageSpatialExtentsPropertyAtom.c
  src/PixelInformationProperty.c
  src/isoiff_cfen.c
  src/JPEGConfigurationAtom.c
)

target_link_libraries(
  libisoiff
  
  LINK_PUBLIC
 
  libisomediafile
)


add_executable(
  isoiff_tool

  src/StringUtils.cpp
  src/Options.cpp
  src/hevc_tool.cpp
  src/avc_tool.cpp
  src/jpeg_tool.cpp
  src/isoiff_tool_main.cpp
)

target_link_libraries(
  isoiff_tool
  
  LINK_PUBLIC
 
  libisoiff
  TLibDecoder
  TLibCommon
  nlohmann_json::nlohmann_json
)

