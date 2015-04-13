# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#                          Stefan.Eilemann@epfl.ch
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

include(Files.cmake)

set(LIVRECORE_LINK_LIBRARIES ${EQ_GL_LIBRARIES} ${COLLAGE_LIBRARIES}
  ${DASH_LIBRARIES} ${Boost_LIBRARIES} ${OPENGL_glu_LIBRARY}
  ${GLEW_MX_LIBRARIES})

common_library(LivreCore)

if(NOT APPLE)
  set_target_properties(LivreCore PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
  add_definitions(-DGL3_PROTOTYPES)
endif()
