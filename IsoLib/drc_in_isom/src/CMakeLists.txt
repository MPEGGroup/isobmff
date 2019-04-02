set(DRCREFSWPATH ../../../external/Part04-Dynamic_Range_Control/trunk)

include_directories(
  ${DRCREFSWPATH}/tools/readonlybitbuf/include
  ${DRCREFSWPATH}/tools/writeonlybitbuf/include
  ${DRCREFSWPATH}/tools/wavIO/include
  ${DRCREFSWPATH}/modules/uniDrcModules/uniDrcBitstreamDecoderLib/include
  ${DRCREFSWPATH}/modules/uniDrcModules/uniDrcCommon
)

include_directories(../../libisomediafile/src)
include_directories(
  # Linux
  $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/linux>

  # Windows
  $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/w32>

  # Mac
  $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/macosx>
)

set(DRC_SOURCE	
  DRCAtoms.c
  DRCAudioSampleEntryAtom.c
  DRCCoefficientBasicAtom.c
  DRCCoefficientUniDRCAtom.c
  DRCData.c
  DRCInstructionsBasicAtom.c
  DRCInstructionsUniDRCAtom.c
  DRCUniDrcSampleEntryAtom.c
  FileFormatDrcToInStreamDrc.c
  InStreamDrcToFileFormatDrc.c
  Logger.c
  Testing.c
  WAVData.c
)

add_executable(DRC_to_MP4 ${DRC_SOURCE} DRC_to_MP4.c)
set_property(TARGET DRC_to_MP4 PROPERTY C_STANDARD 99)

add_executable(MP4_to_DRC ${DRC_SOURCE} MP4_to_DRC.c)
set_property(TARGET MP4_to_DRC PROPERTY C_STANDARD 99)

target_link_libraries(
  DRC_to_MP4
  
  LINK_PUBLIC
  
  m
  libisomediafile
  libreadonlybitbuf
  libwriteonlybitbuf
  libwavIO
  libuniDrcBitstreamDecoderLib
)

target_link_libraries(
  MP4_to_DRC
  
  LINK_PUBLIC
 
  m 
  libisomediafile
  libreadonlybitbuf
  libwriteonlybitbuf
  libwavIO
  libuniDrcBitstreamDecoderLib
)

