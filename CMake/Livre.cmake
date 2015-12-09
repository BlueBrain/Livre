
if(WIN32)
  return()
endif()
set(LIVRE_PACKAGE_VERSION 0.2.0)
set(LIVRE_REPO_URL https://github.com/BlueBrain/Livre)
set(LIVRE_DEPENDS BBPTestData OpenMP Tuvok VTune GLEW_MX zeq FlatBuffers
  RESTBridge LibJpegTurbo
  REQUIRED Boost Collage Equalizer Lunchbox OpenGL PNG Qt5Core Qt5OpenGL
           Qt5Widgets Threads)
set(LIVRE_BOOST_COMPONENTS
  "filesystem program_options thread system regex unit_test_framework")
set(LIVRE_DEB_DEPENDS libqt5opengl5-dev)
set(LIVRE_SUBPROJECT ON)

if(CI_BUILD_COMMIT)
  set(LIVRE_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(LIVRE_REPO_TAG master)
endif()
set(LIVRE_FORCE_BUILD ON)
set(LIVRE_SOURCE ${CMAKE_SOURCE_DIR})
