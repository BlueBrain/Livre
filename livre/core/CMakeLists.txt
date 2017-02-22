
# Copyright (c) 2011-2017, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#                          Stefan.Eilemann@epfl.ch
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

set(LIVRECORE_PUBLIC_HEADERS
  configuration/Configuration.h
  configuration/Parameters.h
  data/DataSource.h
  data/DataSourcePlugin.h
  data/LODNode.h
  data/MemoryUnit.h
  data/NodeId.h
  data/VolumeInformation.h
  render/GLContext.h
  render/GLSLShaders.h
  render/TransferFunction1D.h
  types.h
  util/Utilities.h
)

set(LIVRECORE_HEADERS
  cache/Cache.h
  cache/CacheObject.h
  cache/CacheStatistics.h
  pipeline/Executable.h
  pipeline/Filter.h
  pipeline/FutureMap.h
  pipeline/InputPort.h
  pipeline/OutputPort.h
  pipeline/PipeFilter.h
  pipeline/Pipeline.h
  pipeline/PortData.h
  pipeline/FuturePromise.h
  pipeline/PromiseMap.h
  pipeline/SimpleExecutor.h
  pipeline/Workers.h
  render/FrameInfo.h
  render/Frustum.h
  render/Renderer.h
  render/SelectVisibles.h
  render/TexturePool.h
  render/TextureState.h
  util/FrameUtils.h
  util/ThreadClock.h
  visitor/DFSTraversal.h
  visitor/NodeVisitor.h
  visitor/DataSourceVisitor.h
  visitor/DFSTraversal.h
  visitor/VisitState.h)

set(LIVRECORE_SOURCES
  cache/Cache.cpp
  cache/CacheObject.cpp
  cache/CacheStatistics.cpp
  configuration/Configuration.cpp
  configuration/Parameters.cpp
  data/LODNode.cpp
  data/MemoryUnit.cpp
  data/NodeId.cpp
  data/DataSource.cpp
  data/DataSourcePlugin.cpp
  data/VolumeInformation.cpp
  pipeline/Executable.cpp
  pipeline/FutureMap.cpp
  pipeline/InputPort.cpp
  pipeline/OutputPort.cpp
  pipeline/PipeFilter.cpp
  pipeline/Pipeline.cpp
  pipeline/FuturePromise.cpp
  pipeline/PromiseMap.cpp
  pipeline/SimpleExecutor.cpp
  pipeline/Workers.cpp
  render/FrameInfo.cpp
  render/Frustum.cpp
  render/GLContext.cpp
  render/GLSLShaders.cpp
  render/Renderer.cpp
  render/SelectVisibles.cpp
  render/TexturePool.cpp
  render/TextureState.cpp
  render/TransferFunction1D.cpp
  util/FrameUtils.cpp
  util/ThreadClock.cpp
  util/Utilities.cpp
  visitor/DataSourceVisitor.cpp
  visitor/DFSTraversal.cpp
  visitor/VisitState.cpp)

set(LIVRECORE_LINK_LIBRARIES
  PUBLIC ${Boost_LIBRARIES} Collage Lexis Lunchbox vmmlib ZeroBuf
  PRIVATE Equalizer ${GLEW_MX_LIBRARIES})

set(LIVRECORE_INCLUDE_NAME livre/core)
set(LIVRECORE_NAMESPACE livrecore)
common_library(LivreCore)

if(CMAKE_COMPILER_IS_GCC)
  target_compile_definitions(LivreCore PRIVATE GL3_PROTOTYPES=1)
endif()
