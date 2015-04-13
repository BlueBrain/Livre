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
if(LUNCHBOX_USE_OPENMP)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_subdirectory(core)
add_subdirectory(Lib)
add_subdirectory(Eq)
add_subdirectory(UVF)
add_subdirectory(remote)
