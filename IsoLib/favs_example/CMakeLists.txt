project(favs_example)

include_directories(src)
include_directories(../libisomediafile/src)
include_directories(
    # Linux
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/linux>

    # Windows
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/w32>

    # Mac
    $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/macosx>
)
set(COMMON_SOURCE src/hevc.c src/tools.c)

add_executable(hevc_muxer ${COMMON_SOURCE} src/mux_main.c)

target_link_libraries(
  hevc_muxer
  
  LINK_PUBLIC
  
  $<$<PLATFORM_ID:Linux>:m>
  libisomediafile
)

add_executable(hevc_demuxer ${COMMON_SOURCE} src/demux_main.c)

target_link_libraries(
  hevc_demuxer
  
  LINK_PUBLIC
  
  $<$<PLATFORM_ID:Linux>:m>
  libisomediafile
)
