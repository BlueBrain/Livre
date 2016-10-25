# Copyright (c) 2011-2016, ahmetbilgili@gmail.com
#
# This file is part of Livre <https://github.com/bilgili/Livre>
#

set(LIVREGLRAYCASTPIPELINE_HEADERS GLRaycastRenderer.h
                                   GLRaycasterPipeline.h
                                   GLRenderUploadFilter.h)
set(LIVREGLRAYCASTPIPELINE_SOURCES GLRaycastRenderer.cpp
                                   GLRaycasterPipeline.cpp
                                   GLRenderUploadFilter.cpp)

install(DIRECTORY shaders
        DESTINATION share/Livre
        FILES_MATCHING PATTERN "*.glsl")

set(LIVRE_SHADERS shaders/fragRaycast.glsl
                  shaders/fragTexCopy.glsl
                  shaders/vertRaycast.glsl
                  shaders/vertTexCopy.glsl)

set(SHADER_DIR "${CMAKE_BINARY_DIR}/Livre/share/shaders")

foreach(SHADER ${LIVRE_SHADERS})
  file(COPY ${SHADER} DESTINATION ${SHADER_DIR})
endforeach()

set(LIVREGLRAYCASTPIPELINE_LINK_LIBRARIES PUBLIC LivreLib)
set(LIVREGLRAYCASTPIPELINE_INCLUDE_NAME livre/renderer)
common_library(LivreGLRaycastPipeline)
