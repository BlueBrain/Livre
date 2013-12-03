# Copyright (c)  2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#                2011 Maxim Makhinya <maxmah@gmail.com>
#                2012 David Steiner  <steiner@ifi.uzh.ch>

include(CMakeParseArguments)
include(PurpleInstallPDB)

macro(MASSVOL_ADD_TOOL NAME)
  cmake_parse_arguments(THIS "" "" "SOURCES;HEADERS;LINK_LIBRARIES" ${ARGN})
  set(THIS_TARGET ${NAME})
  source_group(\\ FILES ${THIS_SOURCES} ${THIS_HEADERS})
  add_executable(${THIS_TARGET} ${THIS_SOURCES} ${THIS_HEADERS})
  target_link_libraries(${THIS_TARGET} ${THIS_LINK_LIBRARIES})
  set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${NAME} FOLDER "Tools")
  install(TARGETS ${THIS_TARGET} DESTINATION bin COMPONENT tools)
  purple_install_pdb(${THIS_TARGET} DESTINATION bin COMPONENT tools)
endmacro(MASSVOL_ADD_TOOL NAME)

massvol_add_tool(dataConverter
  HEADERS
    dataConverter/dataConverter.h
    dataConverter/dataManipulation.h
  SOURCES
    dataConverter/dataConverter.cpp
    dataConverter/dataManipulation.cpp
    dataConverter/main.cpp
  LINK_LIBRARIES
    msv
  )
add_dependencies(dataConverter msv)

massvol_add_tool(tensorConverter
  HEADERS
  SOURCES
    tensorConverter/main.cpp
  LINK_LIBRARIES
    msv
  )

massvol_add_tool(histBuilder
  HEADERS
  SOURCES
    histBuilder/main.cpp
    ../volVis/asyncFetcher/compression/tensorCPU.cpp
  LINK_LIBRARIES
    msv
  )

massvol_add_tool(tmpTst
  HEADERS
  SOURCES
    tmpTst/main.cpp
  LINK_LIBRARIES
    msv
  )

massvol_add_tool(treeBuilder
  HEADERS
  SOURCES
    treeBuilder/main.cpp
  LINK_LIBRARIES
    blas lapack
  )

if(CUDA_FOUND)
  include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
  list(APPEND EQUALIZER_DEFINES EQ_USE_CUDA)
  message(STATUS "Using CUDA from: ${CUDA_INCLUDE_DIRS}")
endif()

set(CUDA_COMP_FILES)
if(CUDA_FOUND)
  remove_definitions(${EQ_DEFINITIONS})
  cuda_compile(CUDA_COMP_FILES cudaTest/cudaDecompressor.cu)
  add_definitions(${EQ_DEFINITIONS})
endif()

if(CUDA_FOUND)
  massvol_add_tool(cudaTest
    HEADERS
      cudaTest/cudaDecompressor.h
      cudaTest/cpuDecompressor.h
    SOURCES
      ${CUDA_COMP_FILES}
      cudaTest/main.cpp
      cudaTest/cpuDecompressor.cpp
    LINK_LIBRARIES
      ${CUDA_LIBRARIES}
      msv
    )
endif(CUDA_FOUND)

add_subdirectory(visGUI)
