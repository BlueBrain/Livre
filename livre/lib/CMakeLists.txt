# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/*_generated.h)

set(LIVRELIB_PUBLIC_HEADERS
  types.h
  animation/CameraPath.h
  cache/LRUCache.h
  cache/LRUCachePolicy.h
  cache/TextureCache.h
  cache/TextureDataCache.h
  cache/TextureObject.h
  cache/TextureDataObject.h
  data/MemoryDataSource.h
  configuration/ApplicationParameters.h
  configuration/ClientParameters.h
  configuration/VolumeRendererParameters.h
  pipeline/DataUploadFilter.h
  pipeline/RenderingSetGeneratorFilter.h
  pipeline/RenderFilter.h
  pipeline/RenderPipeFilter.h
  pipeline/VisibleSetGeneratorFilter.h
  render/AvailableSetGenerator.h
  render/ScreenSpaceLODEvaluator.h
  )

set(LIVRELIB_HEADERS
  render/SelectVisibles.h)

set(LIVRELIB_SOURCES
  animation/CameraPath.cpp
  cache/LRUCache.cpp
  cache/LRUCachePolicy.cpp
  cache/TextureCache.cpp
  cache/TextureDataCache.cpp
  cache/TextureObject.cpp
  cache/TextureDataObject.cpp
  data/MemoryDataSource.cpp
  configuration/ApplicationParameters.cpp
  configuration/ClientParameters.cpp
  configuration/VolumeRendererParameters.cpp
  pipeline/DataUploadFilter.cpp
  pipeline/RenderingSetGeneratorFilter.cpp
  pipeline/RenderFilter.cpp
  pipeline/RenderPipeFilter.cpp
  pipeline/VisibleSetGeneratorFilter.cpp
  render/AvailableSetGenerator.cpp
  render/ScreenSpaceLODEvaluator.cpp)

set(LIVRELIB_LINK_LIBRARIES PUBLIC LivreCore PRIVATE Equalizer ${VTUNE_LIBRARIES})

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
