# Copyright (c) BBP/EPFL 2011-2014, Stefan.Eilemann@epfl.ch
#                                   Ahmet.Bilgili@epfl.ch
# Change this number when adding tests to force a CMake run: 6

include(InstallFiles)

if(NOT LIVRE_USE_REMOTE_DATASOURCE)
  list(APPEND EXCLUDE_FROM_TESTS lib/events.cpp)
endif()

# Libraries to link the tests executables with
set(TEST_LIBRARIES LivreCore LivreLib LivreEq ${Boost_LIBRARIES})
set(LIVRE_OPTIONAL_LIBRARIES LivreUVF LivreRemote)
foreach(LIVRE_OPTIONAL_LIBRARY ${LIVRE_OPTIONAL_LIBRARIES})
  if(TARGET ${LIVRE_OPTIONAL_LIBRARY})
    list(APPEND TEST_LIBRARIES ${LIVRE_OPTIONAL_LIBRARY})
  endif()
endforeach()

# Create and install all the tests
if(CMAKE_COMPILER_IS_GNUCXX_PURE)
  set(data-dataSource_LINK_LIBRARIES "-Wl,--no-as-needed")
  set(perf-remoteDataSource_LINK_LIBRARIES "-Wl,--no-as-needed")
endif()
include(CommonCTest)

add_subdirectory(core)
install_files(share/Livre/tests FILES ${TEST_FILES} COMPONENT examples)
