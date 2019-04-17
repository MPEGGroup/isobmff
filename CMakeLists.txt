cmake_minimum_required(VERSION 3.9.2)

project(IsoLib)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  # Update submodules as needed
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
  endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/IsoLib/HM/README")
  message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# Build dependencies

add_subdirectory(external)

# Build isobmff tools

add_subdirectory(IsoLib/libisomediafile)

add_subdirectory(IsoLib/audio_example)

add_subdirectory(IsoLib/drc_in_isom)

add_subdirectory(IsoLib/favs_example)

add_subdirectory(IsoLib/hevc_extractors)

add_subdirectory(IsoLib/ipmp_example)

add_subdirectory(IsoLib/isoiff_tool)

add_subdirectory(IsoLib/pcm_audio_example)
