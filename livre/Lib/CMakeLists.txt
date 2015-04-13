# Copyright (c) 2011-2014, EPFL/Blue Brain Project
#                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

include(Files.cmake)

set(PROJECT_INCLUDE_NAME livre/Lib)
set(LIVRELIB_LINK_LIBRARIES LivreCore ${VTUNE_LIBRARIES})
if(LIVRE_USE_REMOTE_DATASOURCE)
  list(APPEND LIVRELIB_LINK_LIBRARIES ${ZEQ_LIBRARIES})
endif()

common_library(LivreLib)
if(NOT APPLE)
  set_target_properties(LivreLib PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
endif()
