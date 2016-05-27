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
  cache/TextureCache.h
  cache/TextureDataCache.h
  cache/TextureObject.h
  cache/TextureDataObject.h
  configuration/ApplicationParameters.h
  configuration/VolumeRendererParameters.h
  pipeline/DataUploadFilter.h
  pipeline/RenderFilter.h
  pipeline/RenderingSetGeneratorFilter.h
  pipeline/RenderPipeline.h
  pipeline/VisibleSetGeneratorFilter.h
  data/MemoryDataSource.h)

set(LIVRELIB_SOURCES
  ${ZEROBUF_GENERATED_SOURCES}
  animation/CameraPath.cpp
  cache/TextureCache.cpp
  cache/TextureDataCache.cpp
  cache/TextureObject.cpp
  cache/TextureDataObject.cpp
  configuration/ApplicationParameters.cpp
  configuration/VolumeRendererParameters.cpp
  pipeline/DataUploadFilter.cpp
  pipeline/RenderFilter.cpp
  pipeline/RenderingSetGeneratorFilter.cpp
  pipeline/RenderPipeline.cpp
  pipeline/VisibleSetGeneratorFilter.cpp
  data/MemoryDataSource.cpp)

set(LIVRELIB_LINK_LIBRARIES PUBLIC LivreCore
                            PRIVATE Equalizer ${VTUNE_LIBRARIES})

set(LIVRELIB_INCLUDE_NAME livre/lib)
common_library(LivreLib)
