cmake_minimum_required(VERSION 3.9.2)

project(audio_example)

include_directories(../libisomediafile/src)

add_executable(
  playAudioMovieSample
  
  src/playAudioMovieSample.c
)

target_link_libraries(
  playAudioMovieSample
  
  LINK_PUBLIC
  
  libisomediafile
)

add_executable(
  makeAudioMovieSample

  src/makeAudioMovieSample.c
)
target_link_libraries(
  makeAudioMovieSample
  
  LINK_PUBLIC
  
  libisomediafile
)

