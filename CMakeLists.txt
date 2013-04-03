
# Copyright (c) 2011 Maxim Makhinya <maxmah@gmail.com>
#               2012 David Steiner  <steiner@ifi.uzh.ch>
#               2013 Daniel Nachbaur <daniel.nachbaur@epfl.ch>

cmake_minimum_required(VERSION 2.8)
project(Livre)

set(RELEASE_VERSION OFF) # OFF or ABI version
set(LAST_RELEASE 0.1.0)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "0")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)
include(Common)

set(PROJECT_INCLUDE_NAME livre)
include(FindPackages)
include_directories(${CMAKE_CURRENT_LIST_DIR}/libs)

add_subdirectory(libs)
add_subdirectory(tools)
add_subdirectory(volVis)

include(DoxygenRule)
#include(CPackConfig)

set(CTEST_CUSTOM_WARNING_EXCEPTION ${CTEST_CUSTOM_WARNING_EXCEPTION}
  # http://public.kitware.com/Bug/view.php?id=10179
  "ld: warning: duplicate dylib")
include(CTest)

include(PackageConfig)
