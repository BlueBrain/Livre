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

The following bugs were known at release time. Please file a
[Bug Report](https://github.com/BlueBrain/Livre/issues) if you find
any other issue with this release.

* [#62](https://github.com/BlueBrain/Livre/issues/62):
  Missing bricks in first frame after load_equalizer change

## About

The following platforms and build environments are tested:

* Linux: Ubuntu 14.04, RHEL 6.5 (Makefile, x64)

The [API documentation](http://bluebrain.github.io/Livre-0.3/index.html)
can be found on [bluebrain.github.io](http://bluebrain.github.io/).
