#
# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          David Steiner <steiner@ifi.uzh.ch>
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                          Maxim Makhinya  <maxmah@gmail.com>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#


cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(Livre)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common)

include(GitExternal)
include(SubProject)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  git_subproject(Tuvok https://github.com/BlueBrain/Tuvok.git bbp)
endif()

set(VERSION_MAJOR "0")
set(VERSION_MINOR "4")
set(VERSION_PATCH "0")
set(VERSION_ABI 3) # OFF or ABI version

set(PROJECT_namespace livre)
set(PROJECT_INCLUDE_NAME livre/core)
set(LIVRE_DESCRIPTION "Largescale Interactive Volume Rendering Engine")
set(COMMON_PROJECT_DOMAIN ch.epfl.bluebrain)

include(Common)
include(FindPackages)
include(EqGLLibraries)
set(LIVRE_DEPENDENT_LIBRARIES vmmlib Lunchbox dash Equalizer)

add_subdirectory(livre)
add_subdirectory(apps)
add_subdirectory(tests)

include(CPackConfig)

set(DOXYGEN_MAINPAGE_MD README.md)
set(DOXYGEN_EXTRA_INPUT ${PROJECT_SOURCE_DIR}/README.md)
include(DoxygenRule)
