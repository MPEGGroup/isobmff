set(DRCREFSWPATH ../../../external/Part04-Dynamic_Range_Control/trunk)

include_directories(
  ${DRCREFSWPATH}/tools/readonlybitbuf/include
  ${DRCREFSWPATH}/tools/writeonlybitbuf/include
  ${DRCREFSWPATH}/tools/wavIO/include
  ${DRCREFSWPATH}/modules/uniDrcModules/uniDrcBitstreamDecoderLib/include
  ${DRCREFSWPATH}/modules/uniDrcModules/uniDrcCommon
)

include_directories(../../libisomediafile/src)
include_directories(../../libisomediafile/linux)

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

add_executable(MP4_to_DRC ${DRC_SOURCE} MP4_to_DRC.c)

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

