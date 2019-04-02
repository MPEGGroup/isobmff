cmake_minimum_required(VERSION 3.9.2)

project(ipmp_example)

include_directories(../libisomediafile/src)

include_directories(
  # Linux
  $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/linux>

  # Windows
  $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/w32>

  # Mac
  $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/macosx>
)

add_executable(protectAudioMovie src/protectAudioMovie.c)

target_link_libraries(
  protectAudioMovie
  
  LINK_PUBLIC
 
  libisomediafile
)

