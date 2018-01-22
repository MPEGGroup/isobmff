cmake_minimum_required(VERSION 3.4.1)

# CPPFLAGS = -I. -I../src -I../../libisomediafile/linux -I../../libisomediafile/src -O -g -Wall
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O -g -Wall -UNDEBUG")

# CFLAGS = -I.. -I../../src -O -g -Wall -W -ansi -pedantic -Wno-long-long -Wmissing-prototypes -DLINUX
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O -Wall -W -ansi -pedantic -Wno-long-long -Wmissing-prototypes -UNDEBUG")

set (ISOLIB_BASE "${CMAKE_SOURCE_DIR}/../../libisomediafile")
set (EXTRACTOR_SRC "${CMAKE_SOURCE_DIR}/../../hevc_extractors/src")

set(CMAKE_BUILD_TYPE Release)
MESSAGE(INFO " ISOLIB_BASE: " ${ISOLIB_BASE})
MESSAGE(INFO " EXTRACTOR_SRC: " ${EXTRACTOR_SRC})

project(libisomediafile)
include_directories(${ISOLIB_BASE}/macosx)
MESSAGE( STATUS "start compiling libisomediafile" )
set (ISOLIB_SRC ${ISOLIB_BASE}/src)
file(GLOB ISOLIB_FILES "${ISOLIB_SRC}/*.c" "${ISOLIB_BASE}/macosx/*.c")
MESSAGE(INFO " ISOLIB Files: " ${ISOLIB_FILES})
add_library(isomediafile STATIC ${ISOLIB_FILES})

####

MESSAGE( STATUS "start compiling hevc_extractor App" )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

include_directories(${ISOLIB_BASE}/macosx ${ISOLIB_BASE}/src)
include_directories(${EXTRACTOR_SRC})

add_executable (extractor_player ${EXTRACTOR_SRC}/HEVCExtractorReader.cpp ${EXTRACTOR_SRC}/hevc_extractors.cpp)

# Include libraries needed for extractor app
target_link_libraries(extractor_player
                      isomediafile)

