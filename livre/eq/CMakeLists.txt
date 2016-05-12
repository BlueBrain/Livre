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
include(StringifyShaders)
include(Files.cmake)

set(LIVREEQ_SHADERS render/shaders/vertRayCast.glsl
                    render/shaders/fragRayCast.glsl )
stringify_shaders(${LIVREEQ_SHADERS})
list(APPEND LIVREEQ_SOURCES ${SHADER_SOURCES})
include_directories(${PROJECT_BINARY_DIR})

set(LIVREEQ_LINK_LIBRARIES PUBLIC LivreLib Equalizer ZeroBufData)

if(TARGET ZeroEQ)
  list(APPEND LIVREEQ_HEADERS zeroeq/communicator.h)
  list(APPEND LIVREEQ_SOURCES zeroeq/communicator.cpp)
  list(APPEND LIVREEQ_LINK_LIBRARIES PRIVATE ZeroEQ ZeroEQHBP)
endif()

if(LibJpegTurbo_FOUND)
  list(APPEND LIVREEQ_LINK_LIBRARIES PRIVATE ${LibJpegTurbo_LIBRARIES})
endif()

if(RESTBRIDGE_FOUND)
  list(APPEND LIVREEQ_LINK_LIBRARIES PRIVATE RESTBridge)
endif()

set(LIVREEQ_INCLUDE_NAME livre/eq)
set(LIVREEQ_NAMESPACE livreeq)
common_library(LivreEq)
