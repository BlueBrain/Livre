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
  visitor/CollectionTraversal.h
  visitor/DFSTraversal.h
  lunchboxTypes.h
  mathTypes.h
  types.h
)

set(LIVRECORE_HEADERS
  cache/Cache.h
  cache/CacheObject.h
  cache/CacheObjectListener.h
  cache/CacheObjectObserver.h
  cache/CachePolicy.h
  cache/CacheStatistics.h
  cache/EmptyCacheObject.h
  configuration/Configuration.h
  configuration/Parameters.h
  data/VolumeDataSource.h
  events/EventHandler.h
  events/EventHandlerFactory.h
  events/EventInfo.h
  events/EventMapper.h
  maths/maths.h
  maths/Plane.h
  maths/Quantizer.h
  pipeline/Executor.h
  pipeline/Executable.h
  pipeline/Filter.h
  pipeline/FilterInput.h
  pipeline/FilterOutput.h
  pipeline/FunctionFilter.h
  pipeline/InputPort.h
  pipeline/OutputPort.h
  pipeline/PipeFilter.h
  pipeline/Pipeline.h
  pipeline/PortInfo.h
  pipeline/PortData.h
  pipeline/SimpleExecutor.h
  pipeline/Workers.h
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
  render/VisibleSetGenerator.h
  util/ThreadClock.h
  visitor/NodeVisitor.h
  visitor/CollectionTraversal.h
  visitor/DFSTraversal.h
  visitor/RenderNodeVisitor.h
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
  data/VolumeDataSource.cpp
  data/VolumeDataSourcePlugin.cpp
  data/VolumeInformation.cpp
  events/EventMapper.cpp
  maths/maths.cpp
  maths/Plane.cpp
  render/FrameInfo.cpp
  pipeline/FilterInput.cpp
  pipeline/FilterOutput.cpp
  pipeline/FunctionFilter.cpp
  pipeline/InputPort.cpp
  pipeline/OutputPort.cpp
  pipeline/PipeFilter.cpp
  pipeline/Pipeline.cpp
  pipeline/SimpleExecutor.cpp
  pipeline/Workers.cpp
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
  render/VisibleSetGenerator.cpp
  util/ThreadClock.cpp
  util/Utilities.cpp
  visitor/CollectionTraversal.cpp
  visitor/DFSTraversal.cpp
  visitor/RenderNodeVisitor.cpp
  visitor/VisitState.cpp )

set(LIVRECORE_LINK_LIBRARIES
  PUBLIC ${Boost_LIBRARIES} Collage Equalizer Lunchbox vmmlib
  PRIVATE ${GLEW_MX_LIBRARIES})

set(LIVRECORE_INCLUDE_NAME livre/core)
set(LIVRECORE_NAMESPACE livrecore)
common_library(LivreCore)

if(CMAKE_COMPILER_IS_GNUCXX_PURE)
  set_target_properties(LivreCore PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
  add_definitions(-DGL3_PROTOTYPES)
endif()
