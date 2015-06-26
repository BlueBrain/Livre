# Copyright (c) 2011-2014, EPFL/Blue Brain Project
#                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

set(LIVRELIB_PUBLIC_HEADERS
  types.h
  Animation/CameraPath.h
  Cache/LRUCache.h
  Cache/LRUCachePolicy.h
  Cache/RawDataCache.h
  Cache/RawDataObject.h
  Cache/TextureCache.h
  Cache/TextureDataCache.h
  Cache/TextureObject.h
  Cache/TextureDataObject.h
  Data/MemoryDataSource.h
  Configuration/ApplicationParameters.h
  Configuration/RESTParameters.h
  Configuration/VolumeRendererParameters.h
  Render/AvailableSetGenerator.h
  Render/RenderView.h
  Render/ScreenSpaceLODEvaluator.h
  Render/SliceRenderer.h
  Uploaders/DataUploadProcessor.h
  Uploaders/TextureUploadProcessor.h
  Visitor/CollectionTraversal.h
  Visitor/DFSTraversal.h)

set(LIVRELIB_SOURCES
  Animation/CameraPath.cpp
  Cache/LRUCache.cpp
  Cache/LRUCachePolicy.cpp
  Cache/RawDataCache.cpp
  Cache/RawDataObject.cpp
  Cache/TextureCache.cpp
  Cache/TextureDataCache.cpp
  Cache/TextureObject.cpp
  Cache/TextureDataObject.cpp
  Data/MemoryDataSource.cpp
  Configuration/ApplicationParameters.cpp
  Configuration/RESTParameters.cpp
  Configuration/VolumeRendererParameters.cpp
  Render/AvailableSetGenerator.cpp
  Render/RenderView.cpp
  Render/ScreenSpaceLODEvaluator.cpp
  Render/SliceRenderer.cpp
  Uploaders/DataUploadProcessor.cpp
  Uploaders/TextureUploadProcessor.cpp
  Visitor/CollectionTraversal.cpp
  Visitor/DFSTraversal.cpp)

if(LIVRE_USE_REMOTE_DATASOURCE)
  flatbuffers_generate_c_headers(FBS
    zeq/lodNode.fbs
    zeq/memory.fbs
    zeq/uri.fbs
    zeq/volumeInformation.fbs)
  list(APPEND LIVRELIB_HEADERS zeq/events.h)
  list(APPEND LIVRELIB_SOURCES zeq/events.cpp ${FBS_OUTPUTS})
endif()
