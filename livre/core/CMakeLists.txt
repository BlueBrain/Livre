# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#                          Stefan.Eilemann@epfl.ch
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

set(LIVRECORE_PUBLIC_HEADERS
  data/LODNode.h
  data/NodeId.h
  data/MemoryUnit.h
  data/DataSourcePlugin.h
  data/VolumeInformation.h
  render/GLContext.h
  render/GLSLShaders.h
  util/Utilities.h
  lunchboxTypes.h
  mathTypes.h
  types.h
)

set(LIVRECORE_HEADERS
  dashTypes.h
  cache/Cache.h
  cache/CacheObject.h
  cache/CacheStatistics.h
  configuration/Configuration.h
  configuration/Parameters.h
  dash/DashContextTrait.h
  dash/DashRenderNode.h
  dash/DashRenderStatus.h
  dash/DashTree.h
  data/DataSource.h
  dashpipeline/DashConnection.h
  dashpipeline/DashProcessor.h
  dashpipeline/DashProcessorInput.h
  dashpipeline/DashProcessorOutput.h
  events/EventHandler.h
  events/EventHandlerFactory.h
  events/EventInfo.h
  events/EventMapper.h
  maths/maths.h
  maths/Quantizer.h
  pipeline/Connection.h
  pipeline/Processor.h
  pipeline/ProcessorInput.h
  pipeline/ProcessorOutput.h
  render/FrameInfo.h
  render/Frustum.h
  render/Renderer.h
  render/TexturePool.h
  render/TextureState.h
  render/TransferFunction1D.h
  util/FrameUtils.h
  util/ThreadClock.h
  visitor/NodeVisitor.h
  visitor/RenderNodeVisitor.h
  visitor/VisitState.h

  pipeline/AsyncData.h
  pipeline/Executable.h
  pipeline/Filter.h
  pipeline/Future.h
  pipeline/FutureMap.h
  pipeline/InputPort.h
  pipeline/OutputPort.h
  pipeline/PipeFilter.h
  pipeline/Pipeline.h
  pipeline/PortData.h
  pipeline/Promise.h
  pipeline/PromiseMap.h
  pipeline/SimpleExecutor.h
  pipeline/Workers.h )

set(LIVRECORE_SOURCES
  cache/Cache.cpp
  cache/CacheObject.cpp
  cache/CacheStatistics.cpp
  configuration/Configuration.cpp
  configuration/Parameters.cpp
  dash/DashContextTrait.cpp
  dash/DashRenderNode.cpp
  dash/DashRenderStatus.cpp
  dash/DashTree.cpp
  data/LODNode.cpp
  data/MemoryUnit.cpp
  data/NodeId.cpp
  data/DataSource.cpp
  data/DataSourcePlugin.cpp
  data/VolumeInformation.cpp
  dashpipeline/DashConnection.cpp
  dashpipeline/DashProcessor.cpp
  dashpipeline/DashProcessorInput.cpp
  dashpipeline/DashProcessorOutput.cpp
  events/EventMapper.cpp
  pipeline/Processor.cpp
  pipeline/ProcessorInput.cpp
  pipeline/ProcessorOutput.cpp
  render/FrameInfo.cpp
  render/Frustum.cpp
  render/GLContext.cpp
  render/GLSLShaders.cpp
  render/Renderer.cpp
  render/TexturePool.cpp
  render/TextureState.cpp
  render/TransferFunction1D.cpp
  util/FrameUtils.cpp
  util/ThreadClock.cpp
  util/Utilities.cpp
  visitor/RenderNodeVisitor.cpp
  visitor/VisitState.cpp

  pipeline/AsyncData.cpp
  pipeline/Executor.cpp
  pipeline/Future.cpp
  pipeline/FutureMap.cpp
  pipeline/InputPort.cpp
  pipeline/OutputPort.cpp
  pipeline/PipeFilter.cpp
  pipeline/Pipeline.cpp
  pipeline/Promise.cpp
  pipeline/PromiseMap.cpp
  pipeline/SimpleExecutor.cpp
  pipeline/Workers.cpp )

set(LIVRECORE_LINK_LIBRARIES
  PUBLIC ${Boost_LIBRARIES} dash Collage Lexis Lunchbox vmmlib
  PRIVATE Equalizer ${GLEW_MX_LIBRARIES})

set(LIVRECORE_INCLUDE_NAME livre/core)
set(LIVRECORE_NAMESPACE livrecore)
common_library(LivreCore)

if(CMAKE_COMPILER_IS_GCC)
  target_compile_definitions(LivreCore PRIVATE GL3_PROTOTYPES=1)
endif()
