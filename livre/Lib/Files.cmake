# Copyright (c) 2011-2014, EPFL/Blue Brain Project
#                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

set(LIVRELIB_PUBLIC_HEADERS
  types.h
  Algorithm/LODFrustum.h
  Algorithm/Optimizer.h
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
  Configuration/EFPrefetchAlgorithmParameters.h
  Configuration/VolumeRendererParameters.h
  Render/AvailableSetGenerator.h
  Render/SliceRenderer.h
  Render/RenderView.h
  Uploaders/DataUploadProcessor.h
  Uploaders/TextureUploadProcessor.h
  Visitor/CollectionTraversal.h
  Visitor/DFSTraversal.h
  Visitor/LODSelectionVisitor.h)

if(RESTBRIDGE_FOUND)
  list(APPEND LIVRELIB_PUBLIC_HEADERS Configuration/RESTParameters.h)
endif()

set(LIVRELIB_SOURCES
  Algorithm/LODFrustum.cpp
  Algorithm/Optimizer.cpp
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
  Configuration/EFPrefetchAlgorithmParameters.cpp
  Configuration/VolumeRendererParameters.cpp
  Render/AvailableSetGenerator.cpp
  Render/SliceRenderer.cpp
  Render/RenderView.cpp
  Uploaders/DataUploadProcessor.cpp
  Uploaders/TextureUploadProcessor.cpp
  Visitor/CollectionTraversal.cpp
  Visitor/DFSTraversal.cpp
  Visitor/LODSelectionVisitor.cpp)

if(RESTBRIDGE_FOUND)
  list(APPEND LIVRELIB_SOURCES Configuration/RESTParameters.cpp)
endif()

if(LIVRE_USE_REMOTE_DATASOURCE)
  flatbuffers_generate_c_headers(FBS
    zeq/lodNode.fbs
    zeq/memory.fbs
    zeq/uri.fbs
    zeq/volumeInformation.fbs)
  list(APPEND LIVRELIB_HEADERS zeq/events.h)
  list(APPEND LIVRELIB_SOURCES zeq/events.cpp ${FBS_OUTPUTS})
endif()
