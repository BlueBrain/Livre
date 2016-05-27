#
# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          David Steiner <steiner@ifi.uzh.ch>
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                          Maxim Makhinya  <maxmah@gmail.com>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#


cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Livre VERSION 0.6.0)
set(Livre_VERSION_ABI 4)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
                              ${CMAKE_SOURCE_DIR}/CMake/common)

include(GitExternal)

set(LIVRE_namespace livre)
set(LIVRE_INCLUDE_NAME livre/core)
set(LIVRE_DESCRIPTION "Largescale Interactive Volume Rendering Engine (Livre)")
set(LIVRE_MAINTAINER "Blue Brain Project <bbp-open-source@googlegroups.com>")
set(LIVRE_LICENSE LGPL)
set(LIVRE_DEB_DEPENDS libqt5opengl5-dev)
set(COMMON_PROJECT_DOMAIN ch.epfl.bluebrain)

include(Common)

if(APPLE) # WAR otherwise MacPorts X11 (/opt/local) is preferred
  list(REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH /opt/local/lib /usr/local/lib)
  list(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH /opt/local /usr/local)
  link_directories(/opt/X11/lib /usr/X11R6/lib)
  include_directories(SYSTEM /opt/X11/include /usr/X11R6/include)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "Linux")
  git_subproject(Tuvok https://github.com/BlueBrain/Tuvok.git bbp)
endif()

common_find_package(BBPTestData)
common_find_package(Boost REQUIRED COMPONENTS filesystem program_options thread
                                         system regex unit_test_framework)
common_find_package(Collage REQUIRED)
common_find_package(Equalizer REQUIRED)
common_find_package(Lexis REQUIRED)
common_find_package(LibJpegTurbo)
common_find_package(Lunchbox REQUIRED)
common_find_package(Monsteer)
common_find_package(OpenGL REQUIRED)
common_find_package(OpenMP)
common_find_package(Qt5Core)
common_find_package(Qt5OpenGL)
common_find_package(Qt5Widgets)
common_find_package(Threads REQUIRED)
common_find_package(Tuvok)
common_find_package(VTune)
common_find_package(ZeroEQ)
common_find_package(ZeroBuf REQUIRED)
common_find_package_post()

include(EqGLLibraries)
set(LIVRE_DEPENDENT_LIBRARIES vmmlib Lunchbox Equalizer ZeroBuf)

add_definitions(-DBOOST_PROGRAM_OPTIONS_DYN_LINK) # Fix for windows and shared boost.
add_subdirectory(livre)
add_subdirectory(apps)
add_subdirectory(tests)

include(CPackConfig)

set(DOXYGEN_MAINPAGE_MD README.md)
set(DOXYGEN_EXTRA_INPUT ${PROJECT_SOURCE_DIR}/README.md)
include(DoxygenRule)
