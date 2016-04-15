# Copyright (c) 2011-2016, EPFL/Blue Brain Project
#                          David Steiner <steiner@ifi.uzh.ch>
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#                          Marwan Abdellah <marwan.abdellah@epfl.ch>
#                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#
if(NOT ZEROEQ_FOUND)
  message(STATUS "Not building GUI due to missing ZeroEQ dependency")
  return()
endif()

if(NOT QT5WIDGETS_FOUND)
  message(STATUS "Not building GUI due to missing Qt5 dependency")
  return()
endif()

set(LIVREGUI_HEADERS
  AppSettings.h
  Controller.h
  MainWindow.h
  animationController/AnimationController.h
  progress/Progress.h
  renderParametersController/RenderParametersController.h
  tfEditor/ColorMapWidget.h
  tfEditor/HoverPoints.h
  tfEditor/TFWidget.h
  tfEditor/TransferFunctionEditor.h
  tfEditor/Utilities.h
  types.h
)

set(LIVREGUI_SOURCES
  Controller.cpp
  MainWindow.cpp
  MainWindow.ui
  animationController/AnimationController.cpp
  animationController/AnimationController.ui
  livreGUI.cpp
  progress/Progress.cpp
  progress/Progress.ui
  renderParametersController/RenderParametersController.cpp
  renderParametersController/RenderParametersController.ui
  resources/resources.qrc
  tfEditor/ColorMapWidget.cpp
  tfEditor/HoverPoints.cpp
  tfEditor/TFWidget.cpp
  tfEditor/TransferFunctionEditor.cpp
  tfEditor/TransferFunctionEditor.ui
)

set(LIVREGUI_LINK_LIBRARIES LivreCore LivreLib Qt5::Widgets ZeroEQ ZeroEQHBP
  ZeroBuf ZeroBufData)

if(MONSTEER_FOUND)
  list(APPEND LIVREGUI_LINK_LIBRARIES MonsteerQt)
endif()

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/moc_MainWindow.cxx)

add_definitions(${QT_DEFINITIONS} "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED" )

common_application(livreGUI GUI)
