# Copyright (c) 2013 ahmet.bilgili@epfl.ch

# Info: http://www.itk.org/Wiki/CMake:Component_Install_With_CPack

set(CPACK_PACKAGE_VENDOR "bluebrain.epfl.ch")
set(CPACK_PACKAGE_CONTACT "Ahmet Bilgili <ahmet.bilgili@epfl.ch>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Livre ( Large-scale interactive volume rendering engine )")
set(CPACK_PACKAGE_LICENSE "LGPL")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6,
                                  libboost-system,
                                  libboost-program_options,
                                  libboost-date-time,
                                  libboost-thread,
                                  qtbase5-dev,
                                  equalizer,
                                  tuvok")

include(CommonCPack)
