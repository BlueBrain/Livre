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

set(LIVREEQ_LINK_LIBRARIES PUBLIC LivreLib Equalizer)

if(ZEQ_FOUND)
  list(APPEND LIVREEQ_HEADERS zeq/communicator.h)
  list(APPEND LIVREEQ_SOURCES zeq/communicator.cpp)
  list(APPEND LIVREEQ_LINK_LIBRARIES
    PRIVATE zeq zeqHBP ${LibJpegTurbo_LIBRARIES})
endif()

if(RESTBRIDGE_FOUND)
  list(APPEND LIVREEQ_LINK_LIBRARIES PRIVATE RESTBridge)
endif()

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual")
endif()

set(LIVREEQ_INCLUDE_NAME livre/eq)
set(LIVREEQ_NAMESPACE livreeq)
common_library(LivreEq)
if(CMAKE_COMPILER_IS_GNUCXX_PURE)
  set_target_properties(LivreEq PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
endif()
