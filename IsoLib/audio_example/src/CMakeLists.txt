include_directories(../../libisomediafile/src)

add_executable(
  playAudioMovieSample
  
  playAudioMovieSample.c
)

target_link_libraries(
  playAudioMovieSample
  
  LINK_PUBLIC
  
  libisomediafile
)

add_executable(
  makeAudioMovieSample

  makeAudioMovieSample.c
)
target_link_libraries(
  makeAudioMovieSample
  
  LINK_PUBLIC
  
  libisomediafile
)

