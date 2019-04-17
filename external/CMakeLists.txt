cmake_minimum_required(VERSION 3.9.2)

project(external)

# HM

add_subdirectory(HM)

# Part 4

set(SRC Part04-Dynamic_Range_Control/trunk)
set(SRC_READONLYBITBUF ${SRC}/tools/readonlybitbuf)
set(SRC_WRITEONLYBITBUF ${SRC}/tools/writeonlybitbuf)
set(SRC_WAVIO ${SRC}/tools/wavIO)
set(SRC_UNIDRC ${SRC}/modules/uniDrcModules/uniDrcBitstreamDecoderLib)
set(SRC_UNIDRCCOMMON ${SRC}/modules/uniDrcModules/uniDrcCommon)

include_directories(${SRC_READONLYBITBUF}/include)
add_library(libreadonlybitbuf ${SRC_READONLYBITBUF}/src/readonlybitbuf.c)

include_directories(${SRC_WRITEONLYBITBUF}/include)
add_library(libwriteonlybitbuf ${SRC_WRITEONLYBITBUF}/src/writeonlybitbuf.c)

include_directories(${SRC_WAVIO}/include)
add_library(libwavIO ${SRC_WAVIO}/src/wavIO.c)

include_directories(${SRC_UNIDRC}/include ${SRC}/modules/uniDrcModules/uniDrcCommon)
add_library(
  libuniDrcBitstreamDecoderLib
  ${SRC_UNIDRC}/src/uniDrcBitstreamDecoder.h
  ${SRC_UNIDRC}/src/uniDrcBitstreamDecoder.c
  ${SRC_UNIDRC}/src/uniDrcParser.h
  ${SRC_UNIDRC}/src/uniDrcParser.c
  ${SRC_UNIDRCCOMMON}/uniDrc.h
  ${SRC_UNIDRCCOMMON}/uniDrcCommon.h
  ${SRC_UNIDRCCOMMON}/uniDrcTables.h
  ${SRC_UNIDRCCOMMON}/uniDrcTables.c
)
