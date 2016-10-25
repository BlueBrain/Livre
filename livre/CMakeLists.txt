# Copyright (c) 2014-2015, EPFL/Blue Brain Project
#                          Stefan.Eilemann@epfl.ch

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  option(LIVRE_DEBUG_RENDERING "Enable debug output in the rendering pipeline"
    ON)
else()
  option(LIVRE_DEBUG_RENDERING "Enable debug output in the rendering pipeline"
    OFF)
endif()

if(LIVRE_DEBUG_RENDERING)
  add_definitions(-DLIVRE_DEBUG_RENDERING)
endif()

add_subdirectory(core)
add_subdirectory(lib)
add_subdirectory(eq)
