#
# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          David Steiner <steiner@ifi.uzh.ch>
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#
if(NOT Qt5Widgets_FOUND)
  return()
endif()
if(NOT ZEQ_FOUND)
  message(STATUS "Not building GUI due to missing ZeroEQ dependency")
  return()
endif()

set(LIVREGUI_HEADERS
  qtTypes.h
  types.h
  AppSettings.h
  Controller.h
  Editor/TransferFunctionEditor.h
  Editor/GraphCore/AnchorGraphCore.h
  Editor/GraphCore/DoubleGaussGraphCore.h
  Editor/GraphCore/GaussGraphCore.h
  Editor/GraphCore/TransferFunctionGraphCore.h
  Editor/Graphs/AnchorGraph.h
  Editor/Graphs/TransferFunctionGraph.h)

set(LIVREGUI_MOC_HEADERS
  Editor/TransferFunctionEditor.h
  Editor/Graphs/AnchorGraph.h
  Editor/Graphs/TransferFunctionGraph.h)

set(LIVREGUI_SOURCES
  Controller.cpp
  LivreGUI.cpp
  Editor/TransferFunctionEditor.cpp
  Editor/GraphCore/AnchorGraphCore.cpp
  Editor/GraphCore/DoubleGaussGraphCore.cpp
  Editor/GraphCore/GaussGraphCore.cpp
  Editor/GraphCore/TransferFunctionGraphCore.cpp
  Editor/Graphs/AnchorGraph.cpp
  Editor/Graphs/TransferFunctionGraph.cpp)

set(LIVREGUI_RESOURCES resources/resources.qrc)
set(LIVREGUI_UI_FORMS Editor/gui/TransferFunctionEditor.ui)

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/Editor/Graphs/moc_AnchorGraph.cxx
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/Editor/Graphs/moc_TransferFunctionGraph.cxx
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/Editor/moc_TransferFunctionEditor.cxx
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/moc_MainWindow.cxx)

add_definitions(${QT_DEFINITIONS} "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED" )
set(LIVREGUI_LINK_LIBRARIES LivreCore LivreEq ${Qt5Widgets_LIBRARIES})

common_application(livreGUI MACOSX_BUNDLE)
