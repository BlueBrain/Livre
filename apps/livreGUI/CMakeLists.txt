# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          David Steiner <steiner@ifi.uzh.ch>
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#                          Marwan Abdellah <marwan.abdellah@epfl.ch>
#                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#
if(NOT ZEQ_FOUND)
  message(STATUS "Not building GUI due to missing ZeroEQ dependency")
  return()
endif()

if(NOT QT5WIDGETS_FOUND)
  message(STATUS "Not building GUI due to missing Qt5 dependency")
  return()
endif()

set(LIVREGUI_MOC_HEADERS
  MainWindow.h
  animationController/AnimationController.h
  tfEditor/ColorMapWidget.h
  tfEditor/HoverPoints.h
  tfEditor/TFWidget.h
  tfEditor/TransferFunctionEditor.h)

set(LIVREGUI_HEADERS
  ${LIVREGUI_MOC_HEADERS}
  types.h
  AppSettings.h
  Controller.h
  tfEditor/Utilities.h)

set(LIVREGUI_SOURCES
  Controller.cpp
  livreGUI.cpp
  MainWindow.cpp
  animationController/AnimationController.cpp
  tfEditor/ColorMapWidget.cpp
  tfEditor/HoverPoints.cpp
  tfEditor/TFWidget.cpp
  tfEditor/TransferFunctionEditor.cpp)

set(LIVREGUI_RESOURCES resources/resources.qrc)
set(LIVREGUI_UI_FORMS
  MainWindow.ui
  animationController/gui/AnimationController.ui
  tfEditor/gui/TransferFunctionEditor.ui)

set(LIVREGUI_LINK_LIBRARIES LivreCore LivreEq Qt5::Widgets zeq zeqHBP)

if(MONSTEER_FOUND)
  list(APPEND LIVREGUI_LINK_LIBRARIES MonsteerQt)
endif()

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/moc_MainWindow.cxx)

add_definitions(${QT_DEFINITIONS} "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED" )

common_gui_application(livreGUI)
