cmake_minimum_required(VERSION 3.9.2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TESTDATA_PATH "${PROJECT_SOURCE_DIR}/TestData")
configure_file(testdataPath.h.in testdataPath.h @ONLY)

project(Tests LANGUAGES CXX)

include_directories(.)
include_directories(../IsoLib/libisomediafile/src)
include_directories(
  # Linux
  $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_LIST_DIR}/../IsoLib/libisomediafile/linux>
  # Windows
  $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../IsoLib/libisomediafile/w32>
  # Mac
  $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/../IsoLib/libisomediafile/macosx>
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(
  unit_tests

  test_main.cpp
  # test_01_simple.cpp
  # test_sample_groups.cpp
  # test_metadata.cpp
  # test_sample_description.cpp
  test_mebx.cpp
)

target_link_libraries(unit_tests LINK_PUBLIC libisomediafile)