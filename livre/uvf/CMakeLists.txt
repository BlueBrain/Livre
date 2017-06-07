# Copyright (c) 2011-2017, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

if(NOT TARGET Tuvok)
  return()
endif()

set(LIVREUVFSOURCE_HEADERS "UVFDataSource.h")
set(LIVREUVFSOURCE_SOURCES "UVFDataSource.cpp")
set(LIVREUVFSOURCE_LINK_LIBRARIES PRIVATE LivreData Tuvok)
set(LIVREUVFSOURCE_INCLUDE_NAME livre/uvf)

common_library(LivreUVFSource ${LIVREUVFSOURCE_HEADERS} ${LIVREUVFSOURCE_SOURCES})
