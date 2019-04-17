cmake_minimum_required(VERSION 3.9.2)

project(pcm_audio_example)

include_directories(../libisomediafile/src)

include_directories(
  # Linux
  $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/linux>

  # Windows
  $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/w32>

  # Mac
  $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/macosx>
)

set(DRCREFSWPATH ../../external/Part04-Dynamic_Range_Control/trunk)

include_directories(
  ${DRCREFSWPATH}/tools/wavIO/include
)

set(SRC_DIR src)

set(COMMON_SOURCE	
  ${SRC_DIR}/CompareWAVOptions.h
  ${SRC_DIR}/FileFormatToPCM.c
  ${SRC_DIR}/FileFormatToPCM.h
  ${SRC_DIR}/Logger.c
  ${SRC_DIR}/Logger.h
  ${SRC_DIR}/PCMAtoms.c
  ${SRC_DIR}/PCMAtoms.h
  ${SRC_DIR}/PCMFormatData.c
  ${SRC_DIR}/PCMFormatData.h
  ${SRC_DIR}/PCMToFileFormat.c
  ${SRC_DIR}/PCMToFileFormat.h
  ${SRC_DIR}/StringUtils.h
  ${SRC_DIR}/WAVData.c
  ${SRC_DIR}/WAVData.h
)

add_executable(
  WAV_to_MP4
  
  ${COMMON_SOURCE}
  ${SRC_DIR}/WAVToMP4Options.h
  ${SRC_DIR}/WAV_to_MP4.c
)
set_property(TARGET WAV_to_MP4 PROPERTY C_STANDARD 99)

target_link_libraries(
  WAV_to_MP4
  
  LINK_PUBLIC
 
  libisomediafile
  libwavIO
)

add_executable(
  MP4_to_WAV
  
  ${COMMON_SOURCE}
  ${SRC_DIR}/MP4ToWAVOptions.h
  ${SRC_DIR}/MP4_to_WAV.c
)
set_property(TARGET MP4_to_WAV PROPERTY C_STANDARD 99)

target_link_libraries(
  MP4_to_WAV
  
  LINK_PUBLIC
 
  libisomediafile
  libwavIO
)
