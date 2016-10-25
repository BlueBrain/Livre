# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

set(LIVRELIB_PUBLIC_HEADERS
  ${ZEROBUF_GENERATED_HEADERS}
  types.h
  animation/CameraPath.h
  cache/DataObject.h
  cache/HistogramObject.h
  cache/TextureObject.h
  configuration/ApplicationParameters.h
  pipeline/DataUploadFilter.h
  pipeline/HistogramFilter.h
  pipeline/RenderFilter.h
  pipeline/RenderingSetGeneratorFilter.h
  pipeline/TextureUploadFilter.h
  pipeline/VisibleSetGeneratorFilter.h)

set(LIVRELIB_SOURCES
  ${ZEROBUF_GENERATED_SOURCES}
  animation/CameraPath.cpp
  cache/DataObject.cpp
  cache/HistogramObject.cpp
  cache/TextureObject.cpp
  configuration/ApplicationParameters.cpp
  pipeline/DataUploadFilter.cpp
  pipeline/HistogramFilter.cpp
  pipeline/RenderFilter.cpp
  pipeline/RenderingSetGeneratorFilter.cpp
  pipeline/TextureUploadFilter.cpp
  pipeline/VisibleSetGeneratorFilter.cpp)

set(LIVRELIB_LINK_LIBRARIES PUBLIC LivreCore)
set(LIVRELIB_INCLUDE_NAME livre/lib)
common_library(LivreLib)

