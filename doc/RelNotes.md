Release Notes {#Release_Notes}
============

[TOC]

# Introduction {#Introduction}

Livre is an out-of-core, multi-node, multi-gpu, OpenGL volume rendering engine to visualise large
volumetric data sets.

Livre 0.2 is the first release of the tool. It can be retrieved by cloning the [source
code](https://bbpcode.epfl.ch/code/gitweb?p=viz/Livre.git).

## Features {#Features}

Livre provides the following major features to facilitate rendering of large volumetric data sets:
* Visualisation of pre-processed UVF format (
[source code](https://github.com/SCIInstitute/Tuvok.git) ) volume data sets.
* Real-time voxelisation and visualisation of surface meshes using OpenGL 4.2 extensions.
* Real-time voxelisation and visualisation of blue brain morphologies.
* Real-time voxelisation and visualisation of local-field potentials in BBP circuit.
* Multi-node, multi-gpu rendering ( Currently only sort-first rendering )
- - -

# New in this release {#New}

* N/A

## New Features {#NewFeatures}

* N/A

## Enhancements {#Enhancements}

* N/A

## Optimizations {#Optimizations}

* N/A

## Documentation {#Documentation}

* N/A

## Bug Fixes {#Fixes}

* N/A

## Known Bugs {#Bugs}

The following bugs were known at release time. Please file a
[Bug Report](https://bbpteam.epfl.ch/project/issues/browse/LIV) if you find
any other issue with this release.

* LIV-146: Segmentation fault if the octree depth is less than 3 levels.
* LIV-157: Livre renders overlapping LOD nodes from different levels.

- - -

# About {#About}

The following platforms and build
environments are tested:

* Linux: Ubuntu 14.04, RHEL 6.5 (Makefile, x64)

The [API documentation]
(https://bbp.epfl.ch/documentation/code/Livre-0.2/index.html)
can be found on
[bbpteam.epfl.ch]
(https://bbp.epfl.ch/documentation/code/index.html).

- - -

# Errata {#Errata}
