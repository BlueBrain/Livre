#
# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          David Steiner <steiner@ifi.uzh.ch>
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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
  AppSettings.h
  Controller.h
  editor/TransferFunctionEditor.h
  editor/graphcore/AnchorGraphCore.h
  editor/graphcore/DoubleGaussGraphCore.h
  editor/graphcore/GaussGraphCore.h
  editor/graphcore/TransferFunctionGraphCore.h
  editor/graphs/AnchorGraph.h
  editor/graphs/TransferFunctionGraph.h)

set(LIVREGUI_MOC_HEADERS
  editor/TransferFunctionEditor.h
  editor/graphs/AnchorGraph.h
  editor/graphs/TransferFunctionGraph.h)

set(LIVREGUI_SOURCES
  Controller.cpp
  livreGUI.cpp
  editor/TransferFunctionEditor.cpp
  editor/graphcore/AnchorGraphCore.cpp
  editor/graphcore/DoubleGaussGraphCore.cpp
  editor/graphcore/GaussGraphCore.cpp
  editor/graphcore/TransferFunctionGraphCore.cpp
  editor/graphs/AnchorGraph.cpp
  editor/graphs/TransferFunctionGraph.cpp)

set(LIVREGUI_RESOURCES resources/resources.qrc)
set(LIVREGUI_UI_FORMS editor/gui/TransferFunctionEditor.ui)

set(LIVREGUI_LINK_LIBRARIES LivreCore LivreEq Qt5::Widgets)

if(ZEQ_FOUND)
list(APPEND zeq zeqHBP)
endif()

if(ISC_FOUND)
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
list(APPEND LIVREGUI_LINK_LIBRARIES ISC)
list(APPEND CPPCHECK_EXTRA_ARGS
    --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/stimuliController/moc_StimuliController.cxx)
endif()

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/editor/graphs/moc_AnchorGraph.cxx
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/editor/graphs/moc_TransferFunctionGraph.cxx
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/editor/moc_TransferFunctionEditor.cxx
  --suppress=*:${CMAKE_CURRENT_BINARY_DIR}/moc_MainWindow.cxx)

add_definitions(${QT_DEFINITIONS} "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED" )

common_application(livreGUI MACOSX_BUNDLE)
