# Copyright (c) BBP/EPFL 2011-2014, Stefan.Eilemann@epfl.ch
#                                   Ahmet.Bilgili@epfl.ch
# Change this number when adding tests to force a CMake run: 5

include(InstallFiles)

if(NOT LIVRE_USE_REMOTE_DATASOURCE)
  list(APPEND EXCLUDE_FROM_TESTS Lib/events.cpp)
endif()

# Libraries to link the tests executables with
set(TEST_LIBRARIES LivreLib LivreEq ${Boost_LIBRARIES})
set(LIVRE_OPTIONAL_LIBRARIES LivreBBPSDKVox LivreFivox LivreUVF LivreRemote
  LivreMeshVox LivreBBIC)
foreach(LIVRE_OPTIONAL_LIBRARY ${LIVRE_OPTIONAL_LIBRARIES})
  if(TARGET ${LIVRE_OPTIONAL_LIBRARY})
    list(APPEND TEST_LIBRARIES ${LIVRE_OPTIONAL_LIBRARY})
  endif()
endforeach()

# Create and install all the tests
if(CMAKE_COMPILER_IS_GNUCXX_PURE)
  set(Data_dataSource_LINK_LIBRARIES "-Wl,--no-as-needed")
  set(remote_dataSource_LINK_LIBRARIES "-Wl,--no-as-needed")
endif()
include(CommonCTest)
install_files(share/Livre/tests FILES ${TEST_FILES} COMPONENT examples)
