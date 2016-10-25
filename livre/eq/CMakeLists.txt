#
#
#       Ecole Polytechnique Federale de Lausanne
#       Brain Mind Institute,
#       Blue Brain Project
#       (c) 2005-2015. All rights reserved.
#
#       Main Authors: Ahmet Bilgili
#
#
include(EqGLLibraries)
set(LIVREEQ_HEADERS
      Channel.h
      Client.h
      Config.h
      ConnectionDefinitions.h
      Error.h
      events/EqEventHandlerFactory.h
      events/EqEventHandlerFactory.h
      events/EqEventInfo.h
      events/Events.h
      events/handlers/ChannelPointerHandler.h
      events/handlers/KeyboardHandler.h
      FrameData.h
      FrameGrabber.h
      Node.h
      Pipe.h
      render/EqContext.h
      settings/EqApplicationSettings.h
      settings/EqCameraSettings.h
      settings/EqFrameSettings.h
      settings/EqRenderSettings.h
      settings/EqVolumeSettings.h
      Window.h
      types.h)

set( LIVREEQ_SOURCES
      Channel.cpp
      Client.cpp
      Config.cpp
      Error.cpp
      events/EqEventHandler.cpp
      events/EqEventHandlerFactory.cpp
      events/handlers/ChannelPointerHandler.cpp
      events/handlers/KeyboardHandler.cpp
      FrameData.cpp
      FrameGrabber.cpp
      Node.cpp
      Pipe.cpp
      render/EqContext.cpp
      settings/EqApplicationSettings.cpp
      settings/EqCameraSettings.cpp
      settings/EqFrameSettings.cpp
      settings/EqRenderSettings.cpp
      settings/EqVolumeSettings.cpp
      Window.cpp
      )

set(LIVREEQ_LINK_LIBRARIES PUBLIC Lexis LivreLib Equalizer)

if(TARGET ZeroEQ)
  list(APPEND LIVREEQ_HEADERS zeroeq/communicator.h)
  list(APPEND LIVREEQ_SOURCES zeroeq/communicator.cpp)
  list(APPEND LIVREEQ_LINK_LIBRARIES PRIVATE ZeroEQ)
endif()

if(LibJpegTurbo_FOUND)
  list(APPEND LIVREEQ_LINK_LIBRARIES PRIVATE ${LibJpegTurbo_LIBRARIES})
endif()

set(LIVREEQ_INCLUDE_NAME livre/eq)
set(LIVREEQ_NAMESPACE livreeq)
common_library(LivreEq)
