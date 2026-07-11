cmake_minimum_required(VERSION 3.16)

# Name & languages
project(pcm_sniffer LANGUAGES C CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Includes: libisomediafile core + HM
include_directories(
  ../libisomediafile/src

  # HM
  ../HM/source/Lib
  ../HM/source/Lib/TLibCommon
  ../HM/source/Lib/TLibDecoder
)

# Platform-specific include roots for libisomediafile
include_directories(
  # Linux
  $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/linux>
  # Windows
  $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/w32>
  # macOS
  $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/macosx>
)

# common warnings
if (MSVC)
  add_compile_options(/W4)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# ---- Executable ----
add_executable(
  pcm_sniffer
  pcm_sniffer.cpp
)

# Link libraries
target_link_libraries(
  pcm_sniffer

  PRIVATE
    libisomediafile
    TLibDecoder
    TLibCommon
)
