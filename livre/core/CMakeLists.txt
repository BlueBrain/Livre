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
  data/VolumeDataSourcePlugin.h
  data/VolumeInformation.h
  render/GLContext.h
  render/GLSLShaders.h
  render/LODEvaluator.h
  util/Utilities.h
  lunchboxTypes.h
  mathTypes.h
  types.h
)

set(LIVRECORE_HEADERS
  dashTypes.h
  cache/Cache.h
  cache/CacheObject.h
  cache/CacheObjectListener.h
  cache/CacheObjectObserver.h
  cache/CachePolicy.h
  cache/CacheStatistics.h
  cache/EmptyCacheObject.h
  configuration/Configuration.h
  configuration/Parameters.h
  dash/DashContextTrait.h
  dash/DashRenderNode.h
  dash/DashRenderStatus.h
  dash/DashTree.h
  data/LODNodeTrait.h
  data/VolumeDataSource.h
  dashpipeline/DashConnection.h
  dashpipeline/DashProcessor.h
  dashpipeline/DashProcessorInput.h
  dashpipeline/DashProcessorOutput.h
  events/EventHandler.h
  events/EventHandlerFactory.h
  events/EventInfo.h
  events/EventMapper.h
  maths/maths.h
  maths/Plane.h
  maths/Quantizer.h
  pipeline/Connection.h
  pipeline/Processor.h
  pipeline/ProcessorInput.h
  pipeline/ProcessorOutput.h
  render/FrameInfo.h
  render/Frustum.h
  render/GLContextTrait.h
  render/GLWidget.h
  render/RenderBrick.h
  render/Renderer.h
  render/TexturePool.h
  render/TexturePoolFactory.h
  render/TextureState.h
  render/TransferFunction1D.h
  render/View.h
  util/FrameUtils.h
  util/ThreadClock.h
  visitor/NodeVisitor.h
  visitor/RenderNodeVisitor.h
  visitor/VisitState.h)

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
  data/LODNodeTrait.cpp
  data/MemoryUnit.cpp
  data/NodeId.cpp
  data/VolumeDataSource.cpp
  data/VolumeDataSourcePlugin.cpp
  data/VolumeInformation.cpp
  dashpipeline/DashConnection.cpp
  dashpipeline/DashProcessor.cpp
  dashpipeline/DashProcessorInput.cpp
  dashpipeline/DashProcessorOutput.cpp
  events/EventMapper.cpp
  maths/maths.cpp
  maths/Plane.cpp
  pipeline/Processor.cpp
  pipeline/ProcessorInput.cpp
  pipeline/ProcessorOutput.cpp
  render/FrameInfo.cpp
  render/Frustum.cpp
  render/GLContext.cpp
  render/GLSLShaders.cpp
  render/GLContextTrait.cpp
  render/GLWidget.cpp
  render/RenderBrick.cpp
  render/Renderer.cpp
  render/TexturePool.cpp
  render/TexturePoolFactory.cpp
  render/TextureState.cpp
  render/TransferFunction1D.cpp
  render/View.cpp
  util/FrameUtils.cpp
  util/ThreadClock.cpp
  util/Utilities.cpp
  visitor/RenderNodeVisitor.cpp
  visitor/VisitState.cpp)

set(LIVRECORE_LINK_LIBRARIES
  PUBLIC ${Boost_LIBRARIES} dash Collage Lunchbox vmmlib
  PRIVATE Equalizer ${GLEW_MX_LIBRARIES})

set(LIVRECORE_INCLUDE_NAME livre/core)
set(LIVRECORE_NAMESPACE livrecore)
common_library(LivreCore)

if(CMAKE_COMPILER_IS_GNUCXX_PURE)
  set_target_properties(LivreCore PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
  add_definitions(-DGL3_PROTOTYPES)
endif()
