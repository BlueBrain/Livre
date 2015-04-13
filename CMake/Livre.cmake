
if(WIN32)
  return()
endif()
set(LIVRE_PACKAGE_VERSION 0.1.0)
set(LIVRE_REPO_URL ssh://bbpcode.epfl.ch/viz/Livre)
set(LIVRE_SOURCE "${CMAKE_SOURCE_DIR}/src/LivreAlpha")
set(LIVRE_DEPENDS
  BBPTestData FLATIndex OpenMP Tuvok VTune zeq RESTConnector LibJpegTurbo
  REQUIRED Boost Collage dash Equalizer Lunchbox OpenGL PNG Qt4 Threads)
set(LIVRE_BOOST_COMPONENTS "filesystem program_options thread system regex unit_test_framework")
set(LIVRE_QT4_COMPONENTS "QtCore QtGui QtOpenGL")
set(LIVRE_FORCE_BUILD ${CI_BUILD})
set(LIVRE_SUBPROJECT ON)

if(CI_BUILD_COMMIT)
  set(LIVRE_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(LIVRE_REPO_TAG master)
endif()
set(LIVRE_FORCE_BUILD ON)
set(LIVRE_SOURCE ${CMAKE_SOURCE_DIR})