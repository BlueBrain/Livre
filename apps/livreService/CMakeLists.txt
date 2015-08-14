# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

if(NOT LIVRE_USE_REMOTE_DATASOURCE)
  return()
endif()

set(LIVRESERVICE_HEADERS dataService.h)
set(LIVRESERVICE_SOURCES dataSource.cpp)
set(LIVRESERVICE_LINK_LIBRARIES LivreLib zeq)
if(CMAKE_COMPILER_IS_GNUCXX_PURE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

set(LIVRE_OPTIONAL_LIBRARIES LivreBBPSDKVox LivreUVF LivreBBIC)
foreach(LIVRESERVICE_OPTIONAL_LIBRARY ${LIVRESERVICE_OPTIONAL_LIBRARIES})
  if(TARGET ${LIVRESERVICE_OPTIONAL_LIBRARY})
    list(APPEND LIVRESERVICE_LINK_LIBRARIES ${LIVRESERVICE_OPTIONAL_LIBRARY})
  endif()
endforeach()

common_application(livreService)
