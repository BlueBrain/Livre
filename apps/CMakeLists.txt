# Copyright (c) 2014, EPFL/Blue Brain Project
#                     Stefan.Eilemann@epfl.ch
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(livreService)
add_subdirectory(livre)
add_subdirectory(livreGUI)
