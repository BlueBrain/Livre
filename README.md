# Livre
![Livre](doc/images/livre_small.png)

Livre (Large-scale Interactive Volume Rendering Engine) is an out-of-core,
multi-node, multi-gpu, OpenGL volume rendering engine to visualise large
volumetric data sets. It can be retrieved by cloning the [source code]
(https://github.com/BlueBrain/Livre.git).

It provides the following major features to facilitate rendering of large volumetric data sets:
* Visualisation of pre-processed UVF format
  ([source code](https://github.com/SCIInstitute/Tuvok.git)) volume data sets.
* Real-time voxelisation of different data sources (surface meshes, BBP morphologies,
  local-field potentials, etc) through the use of plugins.
* Multi-node, multi-gpu rendering (Currently only sort-first rendering)

To keep track of the changes between releases check the [changelog](doc/Changelog.md).

Contact: bbp-open-source@googlegroups.com

## Known Bugs

Please file a [Bug Report](https://github.com/BlueBrain/Livre/issues) if you find new
issues which have not already been reported in
[Bug Report](https://github.com/BlueBrain/Livre/issues) page. If you find an already reported problem,
please update the corresponding issue with your inputs and outputs.

## About

The following platforms and build environments are tested:

* Linux: Ubuntu 14.04, RHEL 6.5 (Makefile, x64)

The [API documentation](http://bluebrain.github.io/Livre-0.5/index.html)
can be found on [bluebrain.github.io](http://bluebrain.github.io/).

## Building

Building from source is as simple as:

    git clone --recursive https://github.com/BlueBrain/Livre/
    mkdir Livre/Build
    cd Livre/Build
    cmake -GNinja -DCLONE_SUBPROJECTS=ON ..
    ninja

## Funding & Acknowledgment

The development of this software was supported by funding to the Blue Brain Project,
a research center of the École polytechnique fédérale de Lausanne (EPFL), from the
Swiss government’s ETH Board of the Swiss Federal Institutes of Technology.

This project has received funding from the European Union’s FP7-ICT programme
under Grant Agreement No. 604102 (Human Brain Project RUP).

This project has received funding from the European Union's Horizon 2020 Framework
Programme for Research and Innovation under the Specific Grant Agreement No. 720270
(Human Brain Project SGA1).

This project is based upon work supported by the King Abdullah University of Science
and Technology (KAUST) Office of Sponsored Research (OSR) under Award No. OSR-2017-CRG6-3438.

## License

Livre is licensed under the LGPL, unless noted otherwise, e.g., for external dependencies.
See file LICENSE.txt for the full license. External dependencies are either LGPL or
BSD-licensed. See file ACKNOWLEDGEMENTS.txt and AUTHORS.txt for further details.

Copyright (C) 2005-2022 Blue Brain Project/EPFL, Eyescale Software GmbH, Visualization and
Multimedia Lab at University of Zurich, King Abdullah University of Science and Technology
and AUTHORS.txt.

This program is free software: you can redistribute it and/or modify it under the terms of
the GNU Lesser General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this
program.  If not, see <http://www.gnu.org/licenses/>.

