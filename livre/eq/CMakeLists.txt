#
#
#       Ecole Polytechnique Federale de Lausanne
#       Brain Mind Institute,
#       Blue Brain Project
#       (c) 2005-2016. All rights reserved.
#
#       Main Authors: Ahmet Bilgili
#
#

if(APPLE)
  option(LIVRE_USE_GL2 "Use OpenGL 2.0 renderer" ON)
else()
  option(LIVRE_USE_GL2 "Use OpenGL 2.0 renderer" OFF)
endif()

set(LIVREEQ_HEADERS
  Channel.h
  Client.h
  Config.h
  ConnectionDefinitions.h
  Error.h
  EventHandler.h
  FrameData.h
  FrameGrabber.h
  Node.h
  Pipe.h
  Window.h
  render/EqContext.h
  render/RayCastRenderer.h
  serialization.h
  settings/CameraSettings.h
  settings/FrameSettings.h
  settings/RenderSettings.h
  settings/VolumeSettings.h
  types.h
  )

set(LIVREEQ_SOURCES
  Channel.cpp
  Client.cpp
  Config.cpp
  Error.cpp
  EventHandler.cpp
  FrameData.cpp
  FrameGrabber.cpp
  Node.cpp
  Pipe.cpp
  render/EqContext.cpp
  settings/CameraSettings.cpp
  settings/FrameSettings.cpp
  settings/RenderSettings.cpp
  settings/VolumeSettings.cpp
  Window.cpp
  )

include(StringifyShaders)
set(LIVREEQ_SHADERS
  render/shaders/fragAxis.glsl
  render/shaders/fragRayCast.glsl
  render/shaders/fragRayCastGL2.glsl
  render/shaders/fragTexCopy.glsl
  render/shaders/vertAxis.glsl
  render/shaders/vertRayCast.glsl
  render/shaders/vertRayCastGL2.glsl
  render/shaders/vertTexCopy.glsl)

if(LIVRE_USE_GL2)
  list(APPEND LIVREEQ_SOURCES render/RayCastRendererGL2.cpp)
else()
  list(APPEND LIVREEQ_SOURCES render/RayCastRenderer.cpp)
endif()

stringify_shaders(${LIVREEQ_SHADERS})
list(APPEND LIVREEQ_SOURCES ${SHADER_SOURCES})
include_directories(${PROJECT_BINARY_DIR})

set(LIVREEQ_LINK_LIBRARIES PUBLIC Lexis LivreLib Equalizer)

if(TARGET ZeroEQ)
  list(APPEND LIVREEQ_HEADERS zeroeq/communicator.h)
  list(APPEND LIVREEQ_SOURCES zeroeq/communicator.cpp)
  list(APPEND LIVREEQ_LINK_LIBRARIES PRIVATE ZeroEQ)
  if(TARGET ZeroEQHTTP)
    list(APPEND LIVREEQ_LINK_LIBRARIES ZeroEQHTTP)
  endif()
endif()

if(LibJpegTurbo_FOUND)
  list(APPEND LIVREEQ_LINK_LIBRARIES PRIVATE ${LibJpegTurbo_LIBRARIES})
endif()

set(LIVREEQ_INCLUDE_NAME livre/eq)
set(LIVREEQ_NAMESPACE livreeq)
common_library(LivreEq)
