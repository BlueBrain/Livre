# Copyright (c) 2011-2017, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

if(NOT TUVOK_FOUND)
  return()
endif()

set(LIVREUVF_HEADERS "UVFDataSource.h")
set(LIVREUVF_SOURCES "UVFDataSource.cpp")
set(LIVREUVF_LINK_LIBRARIES PRIVATE LivreCore ${TUVOK_LIBRARY})
set(LIVREUVF_INCLUDE_NAME livre/uvf)

common_library(LivreUVF ${LIVREUVF_HEADERS} ${LIVRE_UVF_SOURCES})
