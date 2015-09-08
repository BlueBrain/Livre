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

set(LIVREGUI_HEADERS
  qtTypes.h
  types.h
  animationController/AnimationController.h
  AppSettings.h
  Controller.h
  tfEditor/ColorMapWidget.h
  tfEditor/GradientRenderer.h
  tfEditor/HoverPoints.h
  tfEditor/TFWidget.h
  tfEditor/TransferFunctionEditor.h
  tfEditor/Utilities.h)

set(LIVREGUI_MOC_HEADERS
  animationController/AnimationController.h
  tfEditor/ColorMapWidget.h
  tfEditor/GradientRenderer.h
  tfEditor/HoverPoints.h
  tfEditor/TFWidget.h
  tfEditor/TransferFunctionEditor.h)

set(LIVREGUI_SOURCES
  Controller.cpp
  livreGUI.cpp
  animationController/AnimationController.cpp
  tfEditor/ColorMapWidget.cpp
  tfEditor/GradientRenderer.cpp
  tfEditor/HoverPoints.cpp
  tfEditor/TFWidget.cpp
  tfEditor/TransferFunctionEditor.cpp)

set(LIVREGUI_RESOURCES resources/resources.qrc)
set(LIVREGUI_UI_FORMS 
  animationController/gui/AnimationController.ui 
  tfEditor/gui/TransferFunctionEditor.ui)

set(LIVREGUI_LINK_LIBRARIES LivreCore LivreEq Qt5::Widgets zeq zeqHBP)

if(MONSTEER_FOUND)
list(APPEND LIVREGUI_HEADERS
      stimuliController/nestData.h
      stimuliController/GeneratorModel.h
      stimuliController/GeneratorPropertiesModel.h
      stimuliController/PropertyEditDelegate.h
      stimuliController/StimuliController.h)
list(APPEND LIVREGUI_MOC_HEADERS
      stimuliController/GeneratorModel.h
      stimuliController/GeneratorPropertiesModel.h
      stimuliController/PropertyEditDelegate.h
      stimuliController/StimuliController.h)
list(APPEND LIVREGUI_SOURCES
      stimuliController/nestData.cpp
      stimuliController/GeneratorModel.cpp
      stimuliController/GeneratorPropertiesModel.cpp
      stimuliController/PropertyEditDelegate.cpp
      stimuliController/StimuliController.cpp)
list(APPEND LIVREGUI_UI_FORMS stimuliController/gui/StimuliController.ui)
list(APPEND LIVREGUI_LINK_LIBRARIES Monsteer)
list(APPEND CPPCHECK_EXTRA_ARGS
    --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/stimuliController/moc_StimuliController.cxx)
endif()

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/moc_MainWindow.cxx)

add_definitions(${QT_DEFINITIONS} "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED" )

common_application(livreGUI MACOSX_BUNDLE)
