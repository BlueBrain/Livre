# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/*_generated.h)

include(zerobufGenerateCxx)
zerobuf_generate_cxx(ZEROBUF_GENERATED
  ${OUTPUT_INCLUDE_DIR}/livre/lib/zerobuf zeq/volumeRendererParameters.fbs)

set(LIVRELIB_PUBLIC_HEADERS
  ${ZEROBUF_GENERATED_HEADERS}
  types.h
  animation/CameraPath.h
  cache/TextureCache.h
  cache/TextureDataCache.h
  cache/TextureObject.h
  cache/TextureDataObject.h
  data/MemoryDataSource.h
  configuration/ApplicationParameters.h
  configuration/VolumeRendererParameters.h
  render/AvailableSetGenerator.h
  render/ScreenSpaceLODEvaluator.h
  render/RenderView.h
  uploaders/DataUploadProcessor.h
  uploaders/TextureUploadProcessor.h
  visitor/CollectionTraversal.h
  visitor/DFSTraversal.h)

set(LIVRELIB_HEADERS
  render/SelectVisibles.h)

set(LIVRELIB_SOURCES
  ${ZEROBUF_GENERATED_SOURCES}
  animation/CameraPath.cpp
  cache/TextureCache.cpp
  cache/TextureDataCache.cpp
  cache/TextureObject.cpp
  cache/TextureDataObject.cpp
  data/MemoryDataSource.cpp
  configuration/ApplicationParameters.cpp
  configuration/VolumeRendererParameters.cpp
  render/AvailableSetGenerator.cpp
  render/ScreenSpaceLODEvaluator.cpp
  render/RenderView.cpp
  uploaders/DataUploadProcessor.cpp
  uploaders/TextureUploadProcessor.cpp
  visitor/CollectionTraversal.cpp
  visitor/DFSTraversal.cpp)

set(LIVRELIB_LINK_LIBRARIES PUBLIC LivreCore ZeroBuf
                            PRIVATE Equalizer ${VTUNE_LIBRARIES})

if(LIVRE_USE_REMOTE_DATASOURCE)
  flatbuffers_generate_c_headers(FBS
    zeq/lodNode.fbs
    zeq/memory.fbs
    zeq/uri.fbs
    zeq/volumeInformation.fbs)
  list(APPEND LIVRELIB_HEADERS zeq/events.h)
  list(APPEND LIVRELIB_SOURCES zeq/events.cpp ${FBS_OUTPUTS})
  list(APPEND LIVRELIB_LINK_LIBRARIES PUBLIC zeq)
endif()

set(LIVRELIB_INCLUDE_NAME livre/lib)
common_library(LivreLib)
if(CMAKE_COMPILER_IS_GNUCXX_PURE)
  set_target_properties(LivreLib PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
endif()
