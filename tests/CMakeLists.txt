# Copyright (c) BBP/EPFL 2011-2014, Stefan.Eilemann@epfl.ch
#                                   Ahmet.Bilgili@epfl.ch
# Change this number when adding tests to force a CMake run: 6

include(InstallFiles)

# Libraries to link the tests executables with
set(TEST_LIBRARIES LivreCore LivreLib LivreEq LivreMemoryDataSource
                   LivreRAWDataSource ${Boost_LIBRARIES})

if(TUVOK_FOUND)
  list(APPEND TEST_LIBRARIES LivreUVFDataSource)
  set(UVF_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  set(UVF_DATA_FILE "${UVF_DATA_DIR}/mouse_reduced.uvf")
  file(COPY "uvf/mouse_reduced.uvf" DESTINATION ${UVF_DATA_DIR})
  add_definitions(-DUVF_DATA_FILE=\"${UVF_DATA_FILE}\")
else()
  set(EXCLUDE_FROM_TESTS uvf/uvf.cpp)
endif()

set(RAW_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(RAW_DATA_FILE "${RAW_DATA_DIR}/nucleon.raw")
file(COPY "lib/nucleon.raw" DESTINATION ${RAW_DATA_DIR})

add_definitions(-DRAW_DATA_FILE=\"${RAW_DATA_FILE}\")

set(NRRD_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(NRRD_DATA_FILE "${NRRD_DATA_DIR}/nucleon.nrrd")
file(COPY "lib/nucleon.nrrd" DESTINATION ${NRRD_DATA_DIR})

add_definitions(-DNRRD_DATA_FILE=\"${NRRD_DATA_FILE}\")

# Create and install all the tests
include(CommonCTest)
install_files(share/Livre/tests FILES ${TEST_FILES} COMPONENT examples)
