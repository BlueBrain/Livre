[![Build Status](https://travis-ci.org/bilgili/Livre.svg?branch=master)](https://travis-ci.org/bilgili/Livre)

# Livre

![Livre](doc/images/livre_small.png)

Livre (Large-scale Interactive Volume Rendering Engine) is an out-of-core,
multi-node, multi-gpu, OpenGL volume rendering engine to visualise large
volumetric data sets. Original and long term supported version can be
retrieved by cloning the [source code] (https://github.com/BlueBrain/Livre.git).

This version of Livre will be mainly used for developing new algorithms
with more recent versions of OpenGL, CUDA, Vulkan etc ( may be very unstable ) and
is not meant to be alternative to original Livre that is developed by
the Blue Brain team.

It provides the following major features to facilitate rendering of large volumetric data sets:
* Visualisation of pre-processed UVF format
  ([source code](https://github.com/SCIInstitute/Tuvok.git)) volume data sets.
* Multi-node, multi-gpu rendering (Currently only sort-first rendering)

To keep track of the changes between releases check the [changelog](doc/Changelog.md).

Contact: bbp-open-source@googlegroups.com

## Known Bugs

Please file a [Bug Report](https://github.com/bilgili/Livre/issues) if you find new
issues which have not already been reported in
[Bug Report](https://github.com/bilgili/Livre/issues) page. If you find an already reported problem,
please update the corresponding issue with your inputs and outputs.

## About

The following platforms and build environments are tested:

* Linux: Ubuntu 14.04, RHEL 6.5 (Makefile, x64)

