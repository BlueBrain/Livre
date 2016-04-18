# Copyright (c) BBP/EPFL 2011-2014, Stefan.Eilemann@epfl.ch
#                                   Ahmet.Bilgili@epfl.ch
# Change this number when adding tests to force a CMake run: 6

include(InstallFiles)

# Libraries to link the tests executables with
set(TEST_LIBRARIES LivreCore LivreLib LivreEq ${Boost_LIBRARIES})
set(LIVRE_OPTIONAL_LIBRARIES LivreUVF)
foreach(LIVRE_OPTIONAL_LIBRARY ${LIVRE_OPTIONAL_LIBRARIES})
  if(TARGET ${LIVRE_OPTIONAL_LIBRARY})
    list(APPEND TEST_LIBRARIES ${LIVRE_OPTIONAL_LIBRARY})
  endif()
endforeach()

set(UVF_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(UVF_DATA_FILE "${UVF_DATA_DIR}/mouse_reduced.uvf")
file(COPY "uvf/mouse_reduced.uvf" DESTINATION ${UVF_DATA_DIR})

add_definitions(-DUVF_DATA_FILE=\"${UVF_DATA_FILE}\")

# Create and install all the tests
include(CommonCTest)

add_subdirectory(core)
install_files(share/Livre/tests FILES ${TEST_FILES} COMPONENT examples)
