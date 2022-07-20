project(vvc_base)

include_directories(src)
include_directories(../libisomediafile/src)
include_directories(
    # Windows
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/../libisomediafile/w32>
)
set(COMMON_SOURCE src/vvc.c src/vvc_tools.c)

add_executable(vvc_muxer ${COMMON_SOURCE} src/vvc_mux_main.c)

target_link_libraries(
  vvc_muxer
  
  LINK_PUBLIC
  
  $<$<PLATFORM_ID:Linux>:m>
  libisomediafile
)

add_executable(vvc_demuxer ${COMMON_SOURCE} src/vvc_demux_main.c)

target_link_libraries(
  vvc_demuxer
  
  LINK_PUBLIC
  
  $<$<PLATFORM_ID:Linux>:m>
  libisomediafile
)
