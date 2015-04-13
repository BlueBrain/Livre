# Copyright (c) 2014, EPFL/Blue Brain Project
#                     Stefan.Eilemann@epfl.ch
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

if(NOT LIVRE_USE_REMOTE_DATASOURCE)
  return()
endif()

set(LIVREREMOTE_HEADERS dataSource.h)
set(LIVREREMOTE_SOURCES dataSource.cpp)
set(LIVREREMOTE_LINK_LIBRARIES LivreLib ${ZEQ_LIBRARIES})
set(PROJECT_INCLUDE_NAME livre/remote)

common_library(LivreRemote)
