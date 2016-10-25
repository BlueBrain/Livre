# Copyright (c) 2011-2016, ahmetbilgili@gmail.com
#
# This file is part of Livre <https://github.com/bilgili/Livre>
#

set( LIVRERAWDATASOURCE_HEADERS "RawDataSource.h" )
set( LIVRERAWDATASOURCE_SOURCES "RawDataSource.cpp" )
set( LIVRERAWDATASOURCE_LINK_LIBRARIES PRIVATE LivreCore)
set( LIVRERAWDATASOURCE_INCLUDE_NAME livre/datasources)

common_library(LivreRAWDataSource ${LIVRERAW_HEADERS} ${LIVRE_RAW_SOURCES})
