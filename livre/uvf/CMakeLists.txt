# Copyright (c) 2011-2014, EPFL/Blue Brain Project
#                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

if(NOT TUVOK_FOUND)
  return()
endif()

set( LIVRE_UVF_HEADERS "UVFDataSource.h" )
set( LIVRE_UVF_SOURCES "UVFDataSource.cpp" )

set( LIVRE_UVF_COMPILE_FLAGS "-std=c++0x" )
set( LIVRE_UVF_TARGET_LIBRARIES LivreCore ${TUVOK_LIBRARY} )

set( LIVRE_UVF_INCLUDES ${TUVOK_INCLUDE_DIR} )

include_directories( ${LIVRE_UVF_INCLUDES} )
add_library( LivreUVF SHARED ${LIVRE_UVF_SOURCES} )
target_link_libraries( LivreUVF ${LIVRE_UVF_TARGET_LIBRARIES} )
set_target_properties( LivreUVF PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_ABI} )
set_target_properties( LivreUVF PROPERTIES COMPILE_FLAGS "${LIVRE_UVF_COMPILE_FLAGS}" )
if(NOT APPLE)
  set_target_properties( LivreUVF PROPERTIES LINK_FLAGS "-Wl,--no-undefined" )
endif()
install( TARGETS LivreUVF LIBRARY DESTINATION lib )
install_files(include/livre/UVF FILES ${LIVRE_UVF_HEADERS} )
