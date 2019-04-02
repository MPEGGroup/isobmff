cmake_minimum_required(VERSION 3.9.2)

project(isoiff)

include_directories(../libisomediafile/src)

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
)

