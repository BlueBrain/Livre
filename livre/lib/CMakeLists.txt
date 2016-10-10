# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/*_generated.h)

include(zerobufGenerateCxx)
zerobuf_generate_cxx(ZEROBUF_GENERATED
  ${PROJECT_BINARY_DIR}/include/livre/lib/zerobuf zeroeq/volumeRendererParameters.fbs)

set(LIVRELIB_PUBLIC_HEADERS
  ${ZEROBUF_GENERATED_HEADERS}
  types.h
  animation/CameraPath.h
  cache/DataObject.h
  cache/HistogramObject.h
  cache/TextureObject.h
  data/MemoryDataSource.h
  data/RawDataSource.h
  configuration/ApplicationParameters.h
  configuration/VolumeRendererParameters.h
  pipeline/DataUploadFilter.h
  pipeline/HistogramFilter.h
  pipeline/RenderFilter.h
  pipeline/RenderingSetGeneratorFilter.h
  pipeline/RenderPipeline.h
  pipeline/VisibleSetGeneratorFilter.h
  render/RayCastRenderer.h
  )

set(LIVRELIB_SOURCES
  ${ZEROBUF_GENERATED_SOURCES}
  animation/CameraPath.cpp
  cache/DataObject.cpp
  cache/HistogramObject.cpp
  cache/TextureObject.cpp
  data/MemoryDataSource.cpp
  data/RawDataSource.cpp
  configuration/ApplicationParameters.cpp
  configuration/VolumeRendererParameters.cpp
  pipeline/DataUploadFilter.cpp
  pipeline/HistogramFilter.cpp
  pipeline/RenderFilter.cpp
  pipeline/RenderingSetGeneratorFilter.cpp
  pipeline/RenderPipeline.cpp
  pipeline/VisibleSetGeneratorFilter.cpp
  render/RayCastRenderer.cpp)

install(DIRECTORY render/shaders
        DESTINATION share/Livre
        FILES_MATCHING PATTERN "*.glsl")

set(LIVRE_SHADERS render/shaders/fragRayCast.glsl
                  render/shaders/fragTexCopy.glsl
                  render/shaders/vertRayCast.glsl
                  render/shaders/vertTexCopy.glsl)

set(SHADER_DIR "${CMAKE_BINARY_DIR}/Livre/share/shaders")

foreach(SHADER ${LIVRE_SHADERS})
  file(COPY ${SHADER} DESTINATION ${SHADER_DIR})
endforeach()

set(LIVRELIB_LINK_LIBRARIES PUBLIC LivreCore
                            PRIVATE Equalizer ${VTUNE_LIBRARIES})

set(LIVRELIB_INCLUDE_NAME livre/lib)
common_library(LivreLib)
