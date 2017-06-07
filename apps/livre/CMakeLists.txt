# Copyright (c) 2011-2017, EPFL/Blue Brain Project
#                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
#
# This file is part of Livre <https://github.com/BlueBrain/Livre>
#

set(LIVRE_SOURCES livre.cpp)
set(LIVRE_LINK_LIBRARIES LivreLib LivreEq)

common_application(livre GUI)
