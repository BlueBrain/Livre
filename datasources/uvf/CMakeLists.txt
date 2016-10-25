# Copyright (c) 2011-2014, EPFL/Blue Brain Project
#                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

if(NOT TUVOK_FOUND)
  return()
endif()

set( LIVREUVFDATASOURCE_HEADERS "UVFDataSource.h" )
set( LIVREUVFDATASOURCE_SOURCES "UVFDataSource.cpp" )
set( LIVREUVFDATASOURCE_LINK_LIBRARIES PRIVATE LivreCore ${TUVOK_LIBRARY} )
set( LIVREUVFDATASOURCE_INCLUDE_NAME livre/datasources)

common_library( LivreUVFDataSource ${LIVREUVF_HEADERS} ${LIVREUVF_SOURCES} )
