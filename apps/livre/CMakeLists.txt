# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

set(LIVRE_SOURCES livre.cpp)
set(LIVRE_LINK_LIBRARIES LivreLib LivreEq)
if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-overloaded-virtual")
endif()
if(CMAKE_COMPILER_IS_GNUCXX_PURE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

set(LIVRE_OPTIONAL_LIBRARIES LivreBBPSDKVox LivreUVF LivreRemote LivreBBIC)
foreach(LIVRE_OPTIONAL_LIBRARY ${LIVRE_OPTIONAL_LIBRARIES})
  if(TARGET ${LIVRE_OPTIONAL_LIBRARY})
    list(APPEND LIVRE_LINK_LIBRARIES ${LIVRE_OPTIONAL_LIBRARY})
  endif()
endforeach()

common_application(livre GUI)
