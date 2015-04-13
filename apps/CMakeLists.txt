# Copyright (c) 2014, EPFL/Blue Brain Project
#                     Stefan.Eilemann@epfl.ch
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(livreService)

if(QT_VERSION VERSION_LESS 4.8) # WAR C++11 incompatibility
  add_definitions(-DQT_NO_DEBUG)
endif()
add_subdirectory(livre)
add_subdirectory(livreGUI)
