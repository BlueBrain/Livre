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

set(LIVREEQ_SHADERS Render/Shaders/vertRayCast.glsl
  Render/Shaders/fragRayCast.glsl )
stringify_shaders(${LIVREEQ_SHADERS})
list(APPEND LIVREEQ_SOURCES ${SHADER_SOURCES})
include_directories(${PROJECT_BINARY_DIR})

set(LIVREEQ_LINK_LIBRARIES LivreLib ${EQUALIZER_LIBRARY} ${EQUALIZERFABRIC_LIBRARY} ${QT_LIBRARIES}
  ${ZEQ_LIBRARIES} ${LibJpegTurbo_LIBRARIES} ${RESTCONNECTOR_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual")
set(PROJECT_INCLUDE_NAME livre/Eq)

common_library(LivreEq)
if(NOT APPLE)
  set_target_properties(LivreEq PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
endif()
