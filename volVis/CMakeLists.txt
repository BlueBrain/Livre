
# Copyright (c) 2011 Maxim Makhinya <maxmah@gmail.com>
#               2012 David Steiner  <steiner@ifi.uzh.ch>

include(EqStringifyShaders)

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


# if(CUDA_FOUND)
#   include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
#   message(STATUS "Using CUDA from: ${CUDA_INCLUDE_DIRS}")
# endif()

include(files.cmake)

set(MSV_LIB "${PROJECT_BINARY_DIR}/lib/libmsv.so")

source_group(volVis REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt files.cmake ${VOLVIS_MAIN_SOURCES} )
source_group(asyncFetcher   FILES ${VOLVIS_ASYNC_FETCHER_HEADERS}   ${VOLVIS_ASYNC_FETCHER_SOURCES} )
source_group(EQ             FILES ${VOLVIS_EQ_HEADERS}              ${VOLVIS_EQ_SOURCES}            )
source_group(LB             FILES ${VOLVIS_LB_HEADERS}              ${VOLVIS_LB_SOURCES}            )
source_group(render         FILES ${VOLVIS_RENDERER_HEADERS}        ${VOLVIS_RENDERER_SOURCES}      )
source_group(util           FILES ${VOLVIS_UTIL_HEADERS} )
source_group(shaders        FILES ${VOLVIS_SHADERS} )

function(EQ_ADD_EXAMPLE NAME)
  cmake_parse_arguments(THIS "" "" "SOURCES;HEADERS;SHADERS;LINK_LIBRARIES"
    ${ARGN})

  eq_stringify_shaders(SHADERS ${THIS_SHADERS})

  set(THIS_TARGET ${NAME})
  set(THIS_INPUT)
  source_group(\\ FILES ${THIS_SOURCES} ${THIS_HEADERS} ${THIS_SHADERS})
  add_executable(${THIS_TARGET} MACOSX_BUNDLE ${THIS_SOURCES} ${THIS_HEADERS}
                 ${SHADERS})
  target_link_libraries(${THIS_TARGET} ${EQUALIZER_LIBRARIES}
    ${THIS_LINK_LIBRARIES} ${OPENGL_gl_LIBRARY})

  set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${NAME}
                        FOLDER "Examples")
  if(APPLE AND AGL) # set preference to 32 bit if AGL is available
    update_file(${CMAKE_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in
      ${CMAKE_CURRENT_BINARY_DIR}/MacOSXBundleInfo.plist)
    set_target_properties(${THIS_TARGET} PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/MacOSXBundleInfo.plist)
  endif()

  install(TARGETS ${THIS_TARGET} DESTINATION bin COMPONENT apps)
  install(FILES CMakeLists.txt ${THIS_SOURCES} ${THIS_HEADERS} ${THIS_SHADERS}
    DESTINATION share/Equalizer/examples/${NAME} COMPONENT examples)
endfunction(EQ_ADD_EXAMPLE NAME)

eq_add_example(volVis
  HEADERS
    ${VOLVIS_ASYNC_FETCHER_HEADERS}
    ${VOLVIS_EQ_HEADERS}
    ${VOLVIS_LB_HEADERS}
    ${VOLVIS_RENDERER_HEADERS}
    ${VOLVIS_UTIL_HEADERS}
  SOURCES
    ${VOLVIS_ASYNC_FETCHER_SOURCES}
    ${VOLVIS_EQ_SOURCES}
    ${VOLVIS_LB_SOURCES}
    ${VOLVIS_RENDERER_SOURCES}
    ${VOLVIS_MAIN_SOURCES}
  SHADERS
    ${VOLVIS_SHADERS}
  LINK_LIBRARIES
    ${CARBON_LIBRARY}
#    ${CUDA_LIBRARIES}
    ${LUNCHBOX_LIBRARIES}
    ${MSV_LIB}
    X11
  )



source_group(_volVisTests REGULAR_EXPRESSION .*)
source_group(\\ FILES ${VOLVIS_MAIN_TESTS_SOURCES} )

eq_add_example(_volVisTests
  HEADERS
    ${VOLVIS_ASYNC_FETCHER_HEADERS}
    ${VOLVIS_EQ_HEADERS}
    ${VOLVIS_LB_HEADERS}
    ${VOLVIS_RENDERER_HEADERS}
    ${VOLVIS_UTIL_HEADERS}
  SOURCES
    ${VOLVIS_ASYNC_FETCHER_SOURCES}
    ${VOLVIS_EQ_SOURCES}
    ${VOLVIS_LB_SOURCES}
    ${VOLVIS_RENDERER_SOURCES}
    ${VOLVIS_MAIN_TESTS_SOURCES}
  SHADERS
    ${VOLVIS_SHADERS}
  LINK_LIBRARIES
    ${CARBON_LIBRARY}
#    ${CUDA_LIBRARIES}
    ${LUNCHBOX_LIBRARIES}
    ${MSV_LIB}
    X11
  )

