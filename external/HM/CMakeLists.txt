cmake_minimum_required(VERSION 3.9.2)

project(HM)

set(LIB_DIR ../../IsoLib/HM/source/Lib)
set(SRC_DEC_DIR ${LIB_DIR}/TLibDecoder)
set(SRC_COM_DIR ${LIB_DIR}/TLibCommon)
set(SRC_MD5_DIR ${LIB_DIR}/libmd5)

include_directories(
  ${LIB_DIR}
)

add_library(
  TLibDecoder

  ${SRC_DEC_DIR}/AnnexBread.cpp
  ${SRC_DEC_DIR}/NALread.cpp
  ${SRC_DEC_DIR}/SEIread.cpp
  ${SRC_DEC_DIR}/SyntaxElementParser.cpp
  ${SRC_DEC_DIR}/TDecBinCoderCABAC.cpp
  ${SRC_DEC_DIR}/TDecCAVLC.cpp
  ${SRC_DEC_DIR}/TDecCu.cpp
  ${SRC_DEC_DIR}/TDecEntropy.cpp
  ${SRC_DEC_DIR}/TDecGop.cpp
  ${SRC_DEC_DIR}/TDecSbac.cpp
  ${SRC_DEC_DIR}/TDecSlice.cpp
  ${SRC_DEC_DIR}/TDecTop.cpp
  ${SRC_DEC_DIR}/TDecConformance.cpp
)

add_library(
  TLibCommon

  ${SRC_COM_DIR}/Debug.cpp
  ${SRC_COM_DIR}/TComPicYuv.cpp
  ${SRC_COM_DIR}/TComYuv.cpp
  ${SRC_COM_DIR}/ContextModel.cpp
  ${SRC_COM_DIR}/ContextModel3DBuffer.cpp
  ${SRC_COM_DIR}/SEI.cpp
  ${SRC_COM_DIR}/TComCABACTables.cpp
  ${SRC_COM_DIR}/TComSampleAdaptiveOffset.cpp
  ${SRC_COM_DIR}/TComBitStream.cpp
  ${SRC_COM_DIR}/TComChromaFormat.cpp
  ${SRC_COM_DIR}/TComDataCU.cpp
  ${SRC_COM_DIR}/TComLoopFilter.cpp
  ${SRC_COM_DIR}/TComMotionInfo.cpp
  ${SRC_COM_DIR}/TComPattern.cpp
  ${SRC_COM_DIR}/TComPic.cpp
  ${SRC_COM_DIR}/TComPicSym.cpp
  ${SRC_COM_DIR}/TComPicYuvMD5.cpp
  ${SRC_COM_DIR}/TComPrediction.cpp
  ${SRC_COM_DIR}/TComRdCost.cpp
  ${SRC_COM_DIR}/TComRom.cpp
  ${SRC_COM_DIR}/TComSlice.cpp
  ${SRC_COM_DIR}/TComTrQuant.cpp
  ${SRC_COM_DIR}/TComTU.cpp
  ${SRC_COM_DIR}/TComInterpolationFilter.cpp
  ${SRC_MD5_DIR}/libmd5.c
  ${SRC_COM_DIR}/TComWeightPrediction.cpp
  ${SRC_COM_DIR}/TComRdCostWeightPrediction.cpp
)
