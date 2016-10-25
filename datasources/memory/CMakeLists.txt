# Copyright (c) 2011-2016, ahmetbilgili@gmail.com
#
# This file is part of Livre <https://github.com/bilgili/Livre>
#

set( LIVREMEMORYDATASOURCE_HEADERS "MemoryDataSource.h" )
set( LIVREMEMORYDATASOURCE_SOURCES "MemoryDataSource.cpp" )
set( LIVREMEMORYDATASOURCE_LINK_LIBRARIES PRIVATE LivreCore)
set( LIVREMEMORYDATASOURCE_INCLUDE_NAME livre/datasources)

common_library(LivreMemoryDataSource ${LIVRERAW_HEADERS} ${LIVREMEMORY_SOURCES})
