# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

set(LIVRELIB_PUBLIC_HEADERS
  types.h
  Animation/CameraPath.h
  Cache/LRUCache.h
  Cache/LRUCachePolicy.h
  Cache/TextureCache.h
  Cache/TextureDataCache.h
  Cache/TextureObject.h
  Cache/TextureDataObject.h
  Data/MemoryDataSource.h
  Configuration/ApplicationParameters.h
  Configuration/ClientParameters.h
  Configuration/RESTParameters.h
  Configuration/VolumeRendererParameters.h
  Render/AvailableSetGenerator.h
  Render/ScreenSpaceLODEvaluator.h
  Render/RenderView.h
  Uploaders/DataUploadProcessor.h
  Uploaders/TextureUploadProcessor.h
  Visitor/CollectionTraversal.h
  Visitor/DFSTraversal.h)

set(LIVRELIB_SOURCES
  Animation/CameraPath.cpp
  Cache/LRUCache.cpp
  Cache/LRUCachePolicy.cpp
  Cache/TextureCache.cpp
  Cache/TextureDataCache.cpp
  Cache/TextureObject.cpp
  Cache/TextureDataObject.cpp
  Data/MemoryDataSource.cpp
  Configuration/ApplicationParameters.cpp
  Configuration/ClientParameters.cpp
  Configuration/RESTParameters.cpp
  Configuration/VolumeRendererParameters.cpp
  Render/AvailableSetGenerator.cpp
  Render/ScreenSpaceLODEvaluator.cpp
  Render/RenderView.cpp
  Uploaders/DataUploadProcessor.cpp
  Uploaders/TextureUploadProcessor.cpp
  Visitor/CollectionTraversal.cpp
  Visitor/DFSTraversal.cpp)

set(LIVRELIB_LINK_LIBRARIES LivreCore ${VTUNE_LIBRARIES})

if(LIVRE_USE_REMOTE_DATASOURCE)
  flatbuffers_generate_c_headers(FBS
    zeq/lodNode.fbs
    zeq/memory.fbs
    zeq/uri.fbs
    zeq/volumeInformation.fbs)
  list(APPEND LIVRELIB_HEADERS zeq/events.h)
  list(APPEND LIVRELIB_SOURCES zeq/events.cpp ${FBS_OUTPUTS})
  list(APPEND LIVRELIB_LINK_LIBRARIES ${ZEQ_LIBRARIES})
endif()

set(LIVRELIB_INCLUDE_NAME livre/Lib)
common_library(LivreLib)
if(CMAKE_COMPILER_IS_GNUCXX_PURE)
  set_target_properties(LivreLib PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
endif()
