# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

set(LIVRE_SOURCES livre.cpp)
set(LIVRE_LINK_LIBRARIES LivreLib LivreEq LivreMemoryDataSource LivreRAWDataSource)
set(LIVRE_OPTIONAL_LIBRARIES LivreBBPSDKVox LivreRemote LivreBBIC)

if(TUVOK_FOUND)
  list(APPEND LIVRE_LINK_LIBRARIES LivreUVFDataSource)
endif()

foreach(LIVRE_OPTIONAL_LIBRARY ${LIVRE_OPTIONAL_LIBRARIES})
  if(TARGET ${LIVRE_OPTIONAL_LIBRARY})
    list(APPEND LIVRE_LINK_LIBRARIES ${LIVRE_OPTIONAL_LIBRARY})
  endif()
endforeach()

common_application(livre GUI)
