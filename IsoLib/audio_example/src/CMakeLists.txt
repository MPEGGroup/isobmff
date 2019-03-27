project(audio_example)

include_directories(../../libisomediafile/src)

include_directories(../../libisomediafile/linux)

add_executable(makeAudioMovieSample makeAudioMovieSample.c)

add_executable(playAudioMovieSample playAudioMovieSample.c)

target_link_libraries(makeAudioMovieSample LINK_PUBLIC libisomediafile)
target_link_libraries(playAudioMovieSample LINK_PUBLIC libisomediafile)

