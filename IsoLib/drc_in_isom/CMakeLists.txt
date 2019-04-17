cmake_minimum_required(VERSION 3.9.2)

project(drc_in_isom)

set(DRCREFSWPATH ../../external/Part04-Dynamic_Range_Control/trunk)

include_directories(
  ${DRCREFSWPATH}/tools/readonlybitbuf/include
  ${DRCREFSWPATH}/tools/writeonlybitbuf/include
  ${DRCREFSWPATH}/tools/wavIO/include
  ${DRCREFSWPATH}/modules/uniDrcModules/uniDrcBitstreamDecoderLib/include
  ${DRCREFSWPATH}/modules/uniDrcModules/uniDrcCommon
)

include_directories(../libisomediafile/src)
include_directories(
  # Linux
  $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/linux>

  # Windows
  $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/w32>

  # Mac
  $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/macosx>
)

set(DRC_SOURCE	
  src/DRCAtoms.c
  src/DRCAudioSampleEntryAtom.c
  src/DRCCoefficientBasicAtom.c
  src/DRCCoefficientUniDRCAtom.c
  src/DRCData.c
  src/DRCInstructionsBasicAtom.c
  src/DRCInstructionsUniDRCAtom.c
  src/DRCUniDrcSampleEntryAtom.c
  src/FileFormatDrcToInStreamDrc.c
  src/InStreamDrcToFileFormatDrc.c
  src/Logger.c
  src/Testing.c
  src/WAVData.c
)

add_executable(DRC_to_MP4 ${DRC_SOURCE} src/DRC_to_MP4.c)
set_property(TARGET DRC_to_MP4 PROPERTY C_STANDARD 99)

add_executable(MP4_to_DRC ${DRC_SOURCE} src/MP4_to_DRC.c)
set_property(TARGET MP4_to_DRC PROPERTY C_STANDARD 99)

target_link_libraries(
  DRC_to_MP4
  
  LINK_PUBLIC
  
  $<$<PLATFORM_ID:Linux>:m>
  libisomediafile
  libreadonlybitbuf
  libwriteonlybitbuf
  libwavIO
  libuniDrcBitstreamDecoderLib
)

target_link_libraries(
  MP4_to_DRC
  
  LINK_PUBLIC
 
  $<$<PLATFORM_ID:Linux>:m>
  libisomediafile
  libreadonlybitbuf
  libwriteonlybitbuf
  libwavIO
  libuniDrcBitstreamDecoderLib
)

