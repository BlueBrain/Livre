# Copyright (c) 2011-2015, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#                          Stefan.Eilemann@epfl.ch
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

include(Files.cmake)

set(LIVRECORE_LINK_LIBRARIES ${EQ_GL_LIBRARIES} ${COLLAGE_LIBRARIES}
  ${DASH_LIBRARIES} ${Boost_LIBRARIES} ${GLEW_MX_LIBRARIES})

set(LIVRECORE_INCLUDE_NAME livre/core)
set(LIVRECORE_NAMESPACE livrecore)
common_library(LivreCore)

if(CMAKE_COMPILER_IS_GNUCXX_PURE)
  set_target_properties(LivreCore PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
  add_definitions(-DGL3_PROTOTYPES)
endif()
