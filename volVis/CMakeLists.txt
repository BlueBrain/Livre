
# Copyright (c) 2011 Maxim Makhinya <maxmah@gmail.com>
#               2012 David Steiner  <steiner@ifi.uzh.ch>

include(EqStringifyShaders)

if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DVOLVIS_SOURCE_DIR="${CMAKE_SOURCE_DIR}/")


# if(CUDA_FOUND)
#   include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
#   message(STATUS "Using CUDA from: ${CUDA_INCLUDE_DIRS}")
# endif()

include(files.cmake)

source_group(volVis REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt files.cmake ${VOLVIS_MAIN_SOURCES} )
source_group(asyncFetcher   FILES ${VOLVIS_ASYNC_FETCHER_HEADERS}   ${VOLVIS_ASYNC_FETCHER_SOURCES} )
source_group(EQ             FILES ${VOLVIS_EQ_HEADERS}              ${VOLVIS_EQ_SOURCES}            )
source_group(LB             FILES ${VOLVIS_LB_HEADERS}              ${VOLVIS_LB_SOURCES}            )
source_group(render         FILES ${VOLVIS_RENDERER_HEADERS}        ${VOLVIS_RENDERER_SOURCES}      )
source_group(util           FILES ${VOLVIS_UTIL_HEADERS} )
source_group(shaders        FILES ${VOLVIS_SHADERS} )

eq_stringify_shaders(SHADERS ${VOLVIS_SHADERS})

function(EQ_ADD_EXAMPLE NAME)
  cmake_parse_arguments(THIS "" "" "SOURCES;HEADERS;LINK_LIBRARIES"
    ${ARGN})

  set(THIS_TARGET ${NAME})
  set(THIS_INPUT)
  source_group(\\ FILES ${THIS_SOURCES} ${THIS_HEADERS})
  add_executable(${THIS_TARGET} MACOSX_BUNDLE ${THIS_SOURCES} ${THIS_HEADERS})
  target_link_libraries(${THIS_TARGET} Livre)

  set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${NAME}
                        FOLDER "Examples")
  if(APPLE AND AGL) # set preference to 32 bit if AGL is available
    update_file(${CMAKE_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in
      ${CMAKE_CURRENT_BINARY_DIR}/MacOSXBundleInfo.plist)
    set_target_properties(${THIS_TARGET} PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/MacOSXBundleInfo.plist)
  endif()

  install(TARGETS ${THIS_TARGET} DESTINATION bin COMPONENT apps)
  install(FILES ${THIS_HEADERS}
    DESTINATION include/livre/${NAME} COMPONENT examples)
endfunction()

if(APPLE AND GLX)
  list(APPEND EQUALIZER_LIBRARIES GL)
endif()

add_library(Livre SHARED ${VOLVIS_ALL_HEADERS} ${VOLVIS_ALL_SOURCES} ${SHADERS})
target_link_libraries(Livre ${EQUALIZER_LIBRARIES} ${OPENGL_gl_LIBRARY}
                            ${X11_LIBRARIES} ${CARBON_LIBRARY} dl msv)
install(FILES ${VOLVIS_ALL_HEADERS}
      DESTINATION include/livre COMPONENT dev)
install(TARGETS Livre
      ARCHIVE DESTINATION lib COMPONENT dev
      RUNTIME DESTINATION bin COMPONENT lib
      LIBRARY DESTINATION lib COMPONENT lib)

eq_add_example(volVis SOURCES ${VOLVIS_MAIN_SOURCES})



source_group(_volVisTests REGULAR_EXPRESSION .*)
source_group(\\ FILES ${VOLVIS_MAIN_TESTS_SOURCES} )

eq_add_example(_volVisTests SOURCES ${VOLVIS_MAIN_TESTS_SOURCES})
