# Copyright (c) 2011-2017, EPFL/Blue Brain Project
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

include(zerobufGenerateCxx)
zerobuf_generate_qobject(ZEROBUF_GENERATED ${CMAKE_CURRENT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/livre/lib/zeroeq/volumeRendererParameters.fbs)

set(LIVREGUI_HEADERS
  ${ZEROBUF_GENERATED_HEADERS}
  animationController/AnimationController.h
  AppSettings.h
  clipPlanesController/ClipPlanesController.h
  Controller.h
  MainWindow.h
  progress/Progress.h
  renderParametersController/RenderParametersController.h
  transferFunctionEditor/AlphaWidget.h
  transferFunctionEditor/ColorWidget.h
  transferFunctionEditor/ControlPointsWidget.h
  transferFunctionEditor/RangeWidget.h
  transferFunctionEditor/TransferFunctionEditor.h
  transferFunctionEditor/Utils.h
  types.h
)

set(LIVREGUI_SOURCES
  ${ZEROBUF_GENERATED_SOURCES}
  animationController/AnimationController.cpp
  animationController/AnimationController.ui
  clipPlanesController/ClipPlanesController.cpp
  clipPlanesController/ClipPlanesController.ui
  Controller.cpp
  livreGUI.cpp
  MainWindow.cpp
  MainWindow.ui
  progress/Progress.cpp
  progress/Progress.ui
  renderParametersController/RenderParametersController.cpp
  renderParametersController/RenderParametersController.ui
  transferFunctionEditor/AlphaWidget.cpp
  transferFunctionEditor/ColorWidget.cpp
  transferFunctionEditor/ControlPointsWidget.cpp
  transferFunctionEditor/RangeWidget.cpp
  transferFunctionEditor/RangeWidget.ui
  transferFunctionEditor/resources.qrc
  transferFunctionEditor/TransferFunctionEditor.cpp
  transferFunctionEditor/TransferFunctionEditor.ui
)

set(LIVREGUI_LINK_LIBRARIES Lexis Qt5::Widgets Qt5::QuickWidgets ZeroEQ)

if(TARGET MonsteerQt)
  list(APPEND LIVREGUI_LINK_LIBRARIES MonsteerQt)
endif()

add_definitions(${QT_DEFINITIONS} "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED" )

common_application(livreGUI GUI NOHELP)
